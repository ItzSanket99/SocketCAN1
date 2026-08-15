#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include "../../include/can_messages.h"


#define ECU_TIMEOUT_SECONDS 2


/*
 * Get current monotonic time.
 *
 * CLOCK_MONOTONIC is used for measuring elapsed time.
 */
double get_current_time(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ts.tv_sec +
           (ts.tv_nsec / 1000000000.0);
}


/*
 * Display the current dashboard values.
 */
void display_dashboard(int speed,
                       int rpm,
                       int temperature)
{
    printf("\n");
    printf("--------------------------------\n");
    printf("        Vehicle Dashboard\n");
    printf("--------------------------------\n");
    printf("Speed       : %d km/h\n", speed);
    printf("Engine RPM  : %d rpm\n", rpm);
    printf("Temperature : %d C\n", temperature);
    printf("--------------------------------\n");
}


int main(int argc, char *argv[])
{
    /*
     * ------------------------------------------------
     * Filtering mode
     * ------------------------------------------------
     *
     * 0 = ALL
     * 1 = SPEED
     * 2 = RPM
     */

    int filter_mode = 0;


    if (argc > 1)
    {
        if (strcmp(argv[1], "speed") == 0)
        {
            filter_mode = 1;
        }
        else if (strcmp(argv[1], "rpm") == 0)
        {
            filter_mode = 2;
        }
        else if (strcmp(argv[1], "all") == 0)
        {
            filter_mode = 0;
        }
        else
        {
            printf("Invalid argument.\n");
            printf("Usage:\n");
            printf("  ./dashboard_ecu all\n");
            printf("  ./dashboard_ecu speed\n");
            printf("  ./dashboard_ecu rpm\n");

            return 1;
        }
    }


    /*
     * ------------------------------------------------
     * Create CAN socket
     * ------------------------------------------------
     */

    int socket_fd =
        socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if (socket_fd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }


    /*
     * ------------------------------------------------
     * Find vcan0 interface
     * ------------------------------------------------
     */

    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));

    strcpy(ifr.ifr_name, "vcan0");


    if (ioctl(socket_fd,
              SIOCGIFINDEX,
              &ifr) < 0)
    {
        perror("Interface lookup failed");

        close(socket_fd);

        return 1;
    }


    /*
     * ------------------------------------------------
     * Configure CAN address
     * ------------------------------------------------
     */

    struct sockaddr_can addr;

    memset(&addr, 0, sizeof(addr));

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;


    /*
     * ------------------------------------------------
     * Configure CAN filtering
     * ------------------------------------------------
     */

    if (filter_mode == 1)
    {
        struct can_filter filter;

        filter.can_id = CAN_ID_SPEED;
        filter.can_mask = CAN_SFF_MASK;


        if (setsockopt(socket_fd,
                       SOL_CAN_RAW,
                       CAN_RAW_FILTER,
                       &filter,
                       sizeof(filter)) < 0)
        {
            perror("Speed filter failed");

            close(socket_fd);

            return 1;
        }

        printf("Dashboard filter: SPEED only\n");
    }


    else if (filter_mode == 2)
    {
        struct can_filter filter;

        filter.can_id = CAN_ID_RPM;
        filter.can_mask = CAN_SFF_MASK;


        if (setsockopt(socket_fd,
                       SOL_CAN_RAW,
                       CAN_RAW_FILTER,
                       &filter,
                       sizeof(filter)) < 0)
        {
            perror("RPM filter failed");

            close(socket_fd);

            return 1;
        }

        printf("Dashboard filter: RPM only\n");
    }


    else
    {
        printf("Dashboard filter: ALL messages\n");
    }


    /*
     * ------------------------------------------------
     * Bind socket to vcan0
     * ------------------------------------------------
     */

    if (bind(socket_fd,
             (struct sockaddr *)&addr,
             sizeof(addr)) < 0)
    {
        perror("Socket bind failed");

        close(socket_fd);

        return 1;
    }


    printf("Dashboard ECU started...\n");


    /*
     * ------------------------------------------------
     * Vehicle values
     * ------------------------------------------------
     */

    int speed = 0;
    int rpm = 0;
    int temperature = 0;


    /*
     * ------------------------------------------------
     * Timeout tracking
     * ------------------------------------------------
     */

    double last_speed_time =
        get_current_time();

    int offline_warning = 0;


    /*
     * ------------------------------------------------
     * Main loop
     * ------------------------------------------------
     */

    while (1)
    {
        fd_set readfds;

        FD_ZERO(&readfds);

        FD_SET(socket_fd, &readfds);


        /*
         * Wait for CAN traffic for at most
         * one second.
         */

        struct timeval timeout;

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;


        int result =
            select(socket_fd + 1,
                   &readfds,
                   NULL,
                   NULL,
                   &timeout);


        /*
         * ------------------------------------------------
         * CAN message received
         * ------------------------------------------------
         */

        if (result > 0 &&
            FD_ISSET(socket_fd, &readfds))
        {
            struct can_frame frame;

            int bytes_received =
                read(socket_fd,
                     &frame,
                     sizeof(frame));


            if (bytes_received < 0)
            {
                perror("CAN reception failed");
                break;
            }


            switch (frame.can_id)
            {
                /*
                 * SPEED
                 */

                case CAN_ID_SPEED:

                    if (frame.can_dlc >= 2)
                    {
                        int encoded_speed =
                            ((frame.data[0] << 8) |
                             frame.data[1]);

                        speed =
                            encoded_speed / 10;


                        /*
                         * Speed message acts
                         * as ECU heartbeat.
                         */

                        last_speed_time =
                            get_current_time();

                        offline_warning = 0;


                        /*
                         * Display dashboard only
                         * when Speed is received.
                         */

                        display_dashboard(
                            speed,
                            rpm,
                            temperature
                        );
                    }

                    break;


                /*
                 * RPM
                 */

                case CAN_ID_RPM:

                    if (frame.can_dlc >= 2)
                    {
                        rpm =
                            ((frame.data[0] << 8) |
                             frame.data[1]);
                    }

                    break;


                /*
                 * TEMPERATURE
                 */

                case CAN_ID_TEMPERATURE:

                    if (frame.can_dlc >= 1)
                    {
                        temperature =
                            frame.data[0];
                    }

                    break;


                /*
                 * UNKNOWN MESSAGE
                 */

                default:

                    printf("\n");
                    printf("Unknown CAN ID: 0x%03X\n",
                           frame.can_id);

                    break;
            }
        }


        /*
         * ------------------------------------------------
         * Check Vehicle ECU timeout
         * ------------------------------------------------
         */

        double current_time =
            get_current_time();


        double elapsed =
            current_time - last_speed_time;


        if (elapsed >= ECU_TIMEOUT_SECONDS &&
            !offline_warning)
        {
            printf("\n");
            printf("=====================================\n");
            printf(" WARNING: Vehicle ECU Offline\n");
            printf(" No Speed message received for %.0f seconds\n",
                   elapsed);
            printf("=====================================\n");

            offline_warning = 1;
        }
    }


    /*
     * ------------------------------------------------
     * Cleanup
     * ------------------------------------------------
     */

    close(socket_fd);

    return 0;
}
