# SocketCAN Experiments and Learning Challenges

## Challenge 1 — Traffic Observation

Multiple applications can observe the same CAN traffic simultaneously.

The Vehicle ECU transmitted messages through `vcan0` while both the Dashboard ECU and Logger ECU were running. Both applications received the same CAN frames independently.

The Logger ECU did not interfere with Dashboard communication. This demonstrates the broadcast and multi-receiver nature of CAN communication.

## Challenge 2 — Message Filtering

SocketCAN raw socket filtering was implemented using `struct can_filter` and `setsockopt()`.

Three Dashboard modes were tested:

```text
./dashboard_ecu all
./dashboard_ecu speed
./dashboard_ecu rpm
```

The `all` mode receives all defined messages. The `speed` mode receives only CAN ID `0x100`, while the `rpm` mode receives only CAN ID `0x101`.

Kernel-level filtering prevents unwanted CAN frames from being delivered to the application.

## Challenge 3 — Unknown Message Detection

An additional CAN message using ID `0x200` was transmitted using `cansend`.

The Logger ECU recorded the unknown message because it logs all observed CAN traffic.

The Dashboard ECU received the message but identified it as an unknown CAN ID and ignored it without terminating the application.

This demonstrates that CAN traffic can contain messages that are not currently interpreted by a particular application.

## Challenge 4 — Transmission Rate Study

The Vehicle ECU was tested with different transmission periods.

| Period  | Traffic Rate | Observation                             |
| ------- | ------------ | --------------------------------------- |
| 100 ms  | High         | Frequent updates and faster log growth  |
| 500 ms  | Medium       | Moderate traffic and responsive display |
| 1000 ms | Low          | Slower updates and slower log growth    |

Higher transmission frequency produces more CAN traffic and causes the log file to grow faster. Lower frequency reduces traffic but also reduces update responsiveness.

## Challenge 5 — Node Failure Study

The Vehicle ECU process was terminated while the Dashboard ECU and Logger ECU remained active.

The Dashboard stopped receiving new vehicle messages. The Logger also stopped receiving new messages because no new frames were being transmitted.

The applications themselves remained active, demonstrating that loss of CAN traffic does not automatically terminate receiver applications.

## Challenge 6 — CAN FD Exploration

CAN FD communication was demonstrated using Linux SocketCAN and `struct canfd_frame`.

Classical CAN supports a maximum payload of 8 bytes, while CAN FD supports payloads of up to 64 bytes.

A 20-byte CAN FD frame was successfully transmitted through the virtual CAN interface.

The application required CAN FD socket support using:

```c
setsockopt(socket_fd,
           SOL_CAN_RAW,
           CAN_RAW_FD_FRAMES,
           &enable_canfd,
           sizeof(enable_canfd));
```

CAN FD provides larger payload capacity and supports a faster data phase using Bit Rate Switch (BRS).

## Challenge 7 — Basic Diagnostics

The Dashboard ECU uses the periodic Speed message as a communication heartbeat.

A monotonic timer records the time of the last received Speed message. If no Speed message is received for two seconds, the Dashboard displays:

```text
WARNING: Vehicle ECU Offline
```

When Speed messages resume, the offline condition is cleared.

This provides a simple application-level communication-loss diagnostic.
