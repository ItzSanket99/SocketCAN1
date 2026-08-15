#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/ioctl.h>

#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include "../../include/can_messages.h"


int main(int argc, char *argv[]) {

    /* -------------------------------------------------
       Determine filtering mode
       ------------------------------------------------- */

    int filter_mode = 0;
    /*
        0 = receive all
        1 = speed only
        2 = rpm only
    */

    if (argc > 1) {

        if (strcmp(argv[1], "speed") == 0) {
            filter_mode = 1;
        }
        else if (strcmp(argv[1], "rpm") == 0) {
            filter_mode = 2;
        }
        else if (strcmp(argv[1], "all") == 0) {
            filter_mode = 0;
        }
        else {
            printf("Usage:\n");
            printf("  ./dashboard_ecu all\n");
            printf("  ./dashboard_ecu speed\n");
            printf("  ./dashboard_ecu rpm\n");
            return 1;
        }
    }


    /* -------------------------------------------------
       Create CAN socket
       ------------------------------------------------- */

    int socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if (socket_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }


    /* -------------------------------------------------
       Configure CAN interface
       ------------------------------------------------- */

    struct ifreq ifr;

    strcpy(ifr.ifr_name, "vcan0");

    if (ioctl(socket_fd, SIOCGIFINDEX, &ifr) < 0) {
        perror("Interface lookup failed");
        close(socket_fd);
        return 1;
    }


    struct sockaddr_can addr;

    memset(&addr, 0, sizeof(addr));

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;


    /* -------------------------------------------------
       Apply CAN filter
       ------------------------------------------------- */

    if (filter_mode == 1) {

        struct can_filter filter;

        filter.can_id = CAN_ID_SPEED;
        filter.can_mask = CAN_SFF_MASK;

        if (setsockopt(socket_fd,
                       SOL_CAN_RAW,
                       CAN_RAW_FILTER,
                       &filter,
                       sizeof(filter)) < 0) {

            perror("Speed filter failed");
            close(socket_fd);
            return 1;
        }

        printf("Dashboard filter: SPEED only\n");
    }


    else if (filter_mode == 2) {

        struct can_filter filter;

        filter.can_id = CAN_ID_RPM;
        filter.can_mask = CAN_SFF_MASK;

        if (setsockopt(socket_fd,
                       SOL_CAN_RAW,
                       CAN_RAW_FILTER,
                       &filter,
                       sizeof(filter)) < 0) {

            perror("RPM filter failed");
            close(socket_fd);
            return 1;
        }

        printf("Dashboard filter: RPM only\n");
    }


    else {

        printf("Dashboard filter: ALL messages\n");
    }


    /* -------------------------------------------------
       Bind socket
       ------------------------------------------------- */

    if (bind(socket_fd,
             (struct sockaddr *)&addr,
             sizeof(addr)) < 0) {

        perror("Bind failed");
        close(socket_fd);
        return 1;
    }


    printf("Dashboard ECU started...\n\n");


    /* -------------------------------------------------
       Receive frames
       ------------------------------------------------- */

    while (1) {

        struct can_frame frame;

        int bytes_received =
            read(socket_fd,
                 &frame,
                 sizeof(frame));

        if (bytes_received < 0) {
            perror("CAN reception failed");
            break;
        }


        printf("Received: ID=0x%03X DLC=%d Data=",
               frame.can_id,
               frame.can_dlc);


        for (int i = 0; i < frame.can_dlc; i++) {
            printf("%02X ", frame.data[i]);
        }

        printf("\n");
    }


    close(socket_fd);

    return 0;
}
