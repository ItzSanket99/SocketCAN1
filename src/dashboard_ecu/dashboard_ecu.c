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


    int speed = 0;
    int rpm = 0;
    int temperature = 0;

    printf("\n");
    printf("--------------------------------\n");
    printf("        Vehicle Dashboard\n");
    printf("--------------------------------\n");


    while (1) {

        struct can_frame frame;

        int bytes_received =
            read(socket_fd, &frame, sizeof(frame));

        if (bytes_received < 0) {
            perror("CAN reception failed");
            break;
        }


        switch (frame.can_id) {

            case CAN_ID_SPEED:

                if (frame.can_dlc >= 2) {

                    int encoded_speed =
                        ((frame.data[0] << 8) |
                         frame.data[1]);

                    speed = encoded_speed / 10;
                }

                break;


            case CAN_ID_RPM:

                if (frame.can_dlc >= 2) {

                    rpm =
                        ((frame.data[0] << 8) |
                         frame.data[1]);
                }

                break;


            case CAN_ID_TEMPERATURE:

                if (frame.can_dlc >= 1) {

                    temperature = frame.data[0];
                }

                break;


            default:

                printf("Unknown CAN ID: 0x%03X\n",
                       frame.can_id);

                continue;
        }


        printf("\033[4A");

        printf("--------------------------------\n");
        printf("        Vehicle Dashboard\n");
        printf("--------------------------------\n");
        printf("Speed       : %d km/h\n", speed);
        printf("Engine RPM  : %d rpm\n", rpm);
        printf("Temperature : %d C\n", temperature);
        printf("--------------------------------\n");
    }


    close(socket_fd);

    return 0;
}
