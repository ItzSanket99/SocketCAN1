#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>


int main() {

    int socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if (socket_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }


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


    if (bind(socket_fd,
             (struct sockaddr *)&addr,
             sizeof(addr)) < 0) {

        perror("Bind failed");
        close(socket_fd);
        return 1;
    }


    FILE *log_file = fopen("logs/can_traffic.csv", "a");

    if (log_file == NULL) {
        perror("Unable to open log file");
        close(socket_fd);
        return 1;
    }


    fprintf(log_file,
            "Timestamp,CAN_ID,DLC,Data\n");

    fflush(log_file);


    printf("=====================================\n");
    printf("          Logger ECU Started\n");
    printf("=====================================\n");


    while (1) {

        struct can_frame frame;

        int bytes_received =
            read(socket_fd, &frame, sizeof(frame));

        if (bytes_received < 0) {
            perror("CAN reception failed");
            break;
        }


        /* Get timestamp */

        struct timespec ts;

        clock_gettime(CLOCK_REALTIME, &ts);


        struct tm time_info;

        localtime_r(&ts.tv_sec, &time_info);


        char timestamp[64];

        strftime(timestamp,
                 sizeof(timestamp),
                 "%Y-%m-%d %H:%M:%S",
                 &time_info);


        /* Write CSV entry */

        fprintf(log_file,
                "%s.%03ld,0x%03X,%d,",
                timestamp,
                ts.tv_nsec / 1000000,
                frame.can_id,
                frame.can_dlc);


        for (int i = 0; i < frame.can_dlc; i++) {

            fprintf(log_file,
                    "%02X",
                    frame.data[i]);

            if (i < frame.can_dlc - 1)
                fprintf(log_file, " ");
        }


        fprintf(log_file, "\n");

        fflush(log_file);


        printf("[%s.%03ld] ID=0x%03X DLC=%d\n",
               timestamp,
               ts.tv_nsec / 1000000,
               frame.can_id,
               frame.can_dlc);
    }


    fclose(log_file);
    close(socket_fd);

    return 0;
}
