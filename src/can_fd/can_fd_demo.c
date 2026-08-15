#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/ioctl.h>

#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>


int main(void)
{
    /*
     * ------------------------------------------------
     * Create CAN RAW socket
     * ------------------------------------------------
     */

    int socket_fd =
        socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if (socket_fd < 0)
    {
        perror("CAN socket creation failed");
        return 1;
    }


    /*
     * ------------------------------------------------
     * Enable CAN FD frames
     * ------------------------------------------------
     */

    int enable_canfd = 1;

    if (setsockopt(socket_fd,
                   SOL_CAN_RAW,
                   CAN_RAW_FD_FRAMES,
                   &enable_canfd,
                   sizeof(enable_canfd)) < 0)
    {
        perror("Enabling CAN FD failed");

        close(socket_fd);

        return 1;
    }


    /*
     * ------------------------------------------------
     * Find vcan0
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
     * Bind socket
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


    /*
     * ------------------------------------------------
     * Create CAN FD frame
     * ------------------------------------------------
     */

    struct canfd_frame frame;

    memset(&frame, 0, sizeof(frame));


    /*
     * CAN FD identifier
     */

    frame.can_id = 0x300;


    /*
     * CAN FD payload length
     *
     * 20 bytes deliberately chosen to demonstrate
     * that CAN FD can carry more than Classical CAN.
     */

    frame.len = 20;


    /*
     * Example payload
     */

    for (int i = 0; i < frame.len; i++)
    {
        frame.data[i] = i + 1;
    }


    /*
     * Enable Bit Rate Switch (BRS)
     *
     * This allows CAN FD to use a faster bit rate
     * during the data phase.
     */

    frame.flags = CANFD_BRS;


    /*
     * ------------------------------------------------
     * Transmit CAN FD frame
     * ------------------------------------------------
     */

    int bytes_sent =
        write(socket_fd,
              &frame,
              sizeof(frame));


    if (bytes_sent != sizeof(frame))
    {
        perror("CAN FD transmission failed");

        close(socket_fd);

        return 1;
    }


    /*
     * ------------------------------------------------
     * Display transmitted frame
     * ------------------------------------------------
     */

    printf("\n");
    printf("=====================================\n");
    printf("       CAN FD Transmission\n");
    printf("=====================================\n");

    printf("CAN ID : 0x%03X\n",
           frame.can_id);

    printf("Length : %d bytes\n",
           frame.len);

    printf("Flags  : 0x%02X\n",
           frame.flags);

    printf("Data   : ");

    for (int i = 0; i < frame.len; i++)
    {
        printf("%02X ",
               frame.data[i]);
    }

    printf("\n");

    printf("=====================================\n");


    close(socket_fd);

    return 0;
}
