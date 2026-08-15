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

#include "../../include/can_messages.h"


int main() {

    /* Create CAN socket */
    int socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if (socket_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    /* Get vcan0 interface index */
    struct ifreq ifr;

    strcpy(ifr.ifr_name, "vcan0");

    if (ioctl(socket_fd, SIOCGIFINDEX, &ifr) < 0) {
        perror("Failed to get interface index");
        close(socket_fd);
        return 1;
    }

    /* Configure CAN address */
    struct sockaddr_can addr;

    memset(&addr, 0, sizeof(addr));

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    /* Bind socket to vcan0 */
    if (bind(socket_fd,
             (struct sockaddr *)&addr,
             sizeof(addr)) < 0) {

        perror("Socket bind failed");
        close(socket_fd);
        return 1;
    }

    printf("=====================================\n");
    printf("        Vehicle ECU Started\n");
    printf("=====================================\n");

    srand(time(NULL));

    int speed = 60;
    int rpm = 2500;
    int temperature = 85;
	int delay_ms = 100;

	char *env_rate = getenv("CAN_RATE_MS");

	if (env_rate != NULL) {
    		delay_ms = atoi(env_rate);
	}

	if (delay_ms <= 0) {
    		delay_ms = 100;
	}

	printf("Transmission period: %d ms\n", delay_ms);
	
    while (1) {

        /*
         * Simulate realistic vehicle behavior.
         *
         * Values change gradually rather than randomly
         * jumping between minimum and maximum.
         */

        int speed_change = (rand() % 7) - 3;
        speed += speed_change;

        if (speed < SPEED_MIN)
            speed = SPEED_MIN;

        if (speed > SPEED_MAX)
            speed = SPEED_MAX;


        /* RPM roughly follows vehicle speed */
        rpm = 1000 + (speed * 30) + ((rand() % 201) - 100);

        if (rpm < RPM_MIN)
            rpm = RPM_MIN;

        if (rpm > RPM_MAX)
            rpm = RPM_MAX;


        /* Slowly vary temperature */
        int temp_change = (rand() % 3) - 1;
        temperature += temp_change;

        if (temperature < TEMP_MIN)
            temperature = TEMP_MIN;

        if (temperature > TEMP_MAX)
            temperature = TEMP_MAX;


        struct can_frame frame;

        memset(&frame, 0, sizeof(frame));


        /* ---------------- SPEED ---------------- */

        frame.can_id = CAN_ID_SPEED;
        frame.can_dlc = 2;

        int speed_encoded = speed * 10;

        frame.data[0] = (speed_encoded >> 8) & 0xFF;
        frame.data[1] = speed_encoded & 0xFF;

        if (write(socket_fd, &frame, sizeof(frame)) != sizeof(frame)) {
            perror("Speed transmission failed");
        }


        /* ---------------- RPM ---------------- */

        memset(&frame, 0, sizeof(frame));

        frame.can_id = CAN_ID_RPM;
        frame.can_dlc = 2;

        frame.data[0] = (rpm >> 8) & 0xFF;
        frame.data[1] = rpm & 0xFF;

        if (write(socket_fd, &frame, sizeof(frame)) != sizeof(frame)) {
            perror("RPM transmission failed");
        }


        /* ---------------- TEMPERATURE ---------------- */

        memset(&frame, 0, sizeof(frame));

        frame.can_id = CAN_ID_TEMPERATURE;
        frame.can_dlc = 1;

        frame.data[0] = temperature;

        if (write(socket_fd, &frame, sizeof(frame)) != sizeof(frame)) {
            perror("Temperature transmission failed");
        }


        printf("Speed: %d km/h | RPM: %d | Temperature: %d C\n",
               speed,
               rpm,
               temperature);

       

	usleep(delay_ms * 1000);
    }

    close(socket_fd);

    return 0;
}
