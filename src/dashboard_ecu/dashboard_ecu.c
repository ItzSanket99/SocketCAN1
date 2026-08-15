#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/ioctl.h>

#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>

int main() {

    /* 1. Create a CAN raw socket */
    int socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if (socket_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    /* 2. Get the interface index of vcan0 */
    struct ifreq ifr;

    strcpy(ifr.ifr_name, "vcan0");

    if (ioctl(socket_fd, SIOCGIFINDEX, &ifr) < 0) {
        perror("Failed to get interface index");
        close(socket_fd);
        return 1;
    }

    /* 3. Configure CAN address */
    struct sockaddr_can addr;

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    /* 4. Bind socket to vcan0 */
    if (bind(socket_fd,
             (struct sockaddr *)&addr,
             sizeof(addr)) < 0) {

        perror("Socket bind failed");
        close(socket_fd);
        return 1;
    }

    printf("Dashboard ECU started...\n");
    printf("Waiting for CAN messages...\n\n");

    /* 5. Continuously receive CAN frames */
    while (1) {

        struct can_frame frame;

        int bytes_received = read(socket_fd,
                                  &frame,
                                  sizeof(frame));

        if (bytes_received < 0) {
            perror("CAN frame reception failed");
            break;
        }

        /* 6. Display received frame */
        printf("CAN Message Received\n");
        printf("--------------------\n");

        printf("CAN ID : 0x%03X\n", frame.can_id);
        printf("DLC    : %d\n", frame.can_dlc);

        printf("Data   : ");

        for (int i = 0; i < frame.can_dlc; i++) {
            printf("%02X ", frame.data[i]);
        }

        printf("\n\n");
    }

    /* 7. Close socket */
    close(socket_fd);

    return 0;
}
