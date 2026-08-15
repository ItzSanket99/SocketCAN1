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

    /* 3. Specify the CAN interface */
    struct sockaddr_can addr;

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    /* 4. Bind the socket to vcan0 */
    if (bind(socket_fd,
             (struct sockaddr *)&addr,
             sizeof(addr)) < 0) {

        perror("Socket bind failed");
        close(socket_fd);
        return 1;
    }

    /* 5. Create a CAN frame */
    struct can_frame frame;

    memset(&frame, 0, sizeof(frame));

    frame.can_id = 0x123;
    frame.can_dlc = 4;

    frame.data[0] = 0x11;
    frame.data[1] = 0x22;
    frame.data[2] = 0x33;
    frame.data[3] = 0x44;

    /* 6. Transmit the CAN frame */
    int bytes_sent = write(socket_fd,
                           &frame,
                           sizeof(frame));

    if (bytes_sent != sizeof(frame)) {
        perror("CAN frame transmission failed");
        close(socket_fd);
        return 1;
    }

    printf("CAN frame transmitted successfully\n");
    printf("CAN ID : 0x%03X\n", frame.can_id);
    printf("DLC    : %d\n", frame.can_dlc);
    printf("Data   : ");

    for (int i = 0; i < frame.can_dlc; i++) {
        printf("%02X ", frame.data[i]);
    }

    printf("\n");

    /* 7. Close the socket */
    close(socket_fd);

    return 0;
}
