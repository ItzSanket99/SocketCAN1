#ifndef CAN_MESSAGES_H
#define CAN_MESSAGES_H

/* CAN Message IDs */
#define CAN_ID_SPEED        0x100
#define CAN_ID_RPM          0x101
#define CAN_ID_TEMPERATURE  0x102

/* Vehicle limits */
#define SPEED_MIN       0
#define SPEED_MAX       120

#define RPM_MIN         800
#define RPM_MAX         5000

#define TEMP_MIN        20
#define TEMP_MAX        120

/* Transmission periods in milliseconds */
#define SPEED_PERIOD_MS 100
#define RPM_PERIOD_MS   100
#define TEMP_PERIOD_MS  500

#endif
