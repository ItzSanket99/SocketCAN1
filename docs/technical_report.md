# Exploring SocketCAN: Software-Only CAN/CAN FD Communication on Linux

## 1. Introduction

Controller Area Network (CAN) is widely used in automotive and embedded systems for communication between electronic control units. Development and testing can be difficult when physical CAN hardware is unavailable. Linux provides SocketCAN, which integrates CAN communication into the Linux networking subsystem and allows applications to communicate through standard socket APIs.

This project investigates SocketCAN and implements a software-only CAN network using Linux Virtual CAN (`vcan0`). Three C-based software nodes were developed: Vehicle ECU, Dashboard ECU, and Logger ECU. The project also investigates CAN filtering, communication failure detection, transmission rates, unknown messages, and CAN FD.

## 2. SocketCAN and System Architecture

SocketCAN allows CAN interfaces to be represented as Linux network interfaces. Applications create raw CAN sockets using the Linux socket API and bind them to interfaces such as `vcan0`.

The project architecture is:

```text
                         Linux / Pop!_OS
                              │
                         SocketCAN
                              │
                            vcan0
                              │
              ┌───────────────┼───────────────┐
              │               │               │
              ▼               ▼               ▼
       Vehicle ECU      Dashboard ECU     Logger ECU
            │                  │                │
            └──────── CAN Frames ──────────────┘
```

The Vehicle ECU generates and transmits vehicle data. The Dashboard ECU receives and decodes the messages, while the Logger ECU records all CAN traffic. Multiple applications can receive the same CAN traffic simultaneously.

The use of `vcan0` eliminates the need for physical CAN hardware and allows the complete application to be tested in software.

## 3. CAN Message Definition

The project uses standard 11-bit CAN identifiers.

| CAN ID  | Signal              | DLC | Encoding   | Range        |
| ------- | ------------------- | --: | ---------- | ------------ |
| `0x100` | Vehicle Speed       |   2 | Speed × 10 | 0–120 km/h   |
| `0x101` | Engine RPM          |   2 | Integer    | 800–5000 rpm |
| `0x102` | Coolant Temperature |   1 | Integer    | 20–120 °C    |

Vehicle Speed uses a resolution of 0.1 km/h. RPM is represented as a 16-bit integer and temperature as an 8-bit integer.

For example, a speed of 65 km/h is encoded as 650 (`0x028A`) and transmitted as the two-byte payload `02 8A`.

## 4. Implementation

The ECUs were implemented in C using the Linux SocketCAN RAW socket API. The general communication sequence is:

```text
socket()
   ↓
ioctl() → obtain vcan0 interface index
   ↓
bind()
   ↓
write() / read()
```

The Vehicle ECU periodically generates realistic values and transmits the three defined messages. The Dashboard ECU decodes the received payloads and displays the vehicle information. The Logger ECU records timestamps, identifiers, DLC values, and payload data in CSV format.

SocketCAN filtering was implemented using `struct can_filter` and `setsockopt()`. The Dashboard can receive all messages or only Speed or RPM messages.

## 5. Learning Challenge Results

### Traffic Observation

Multiple applications successfully observed the same CAN traffic. Running the Logger did not prevent the Dashboard from receiving messages.

### Message Filtering

Kernel-level SocketCAN filters were successfully used to receive only selected CAN identifiers. This reduced unnecessary messages delivered to the Dashboard application.

### Unknown Messages

An additional message with CAN ID `0x200` was introduced. The Logger recorded it, while the Dashboard identified it as an unknown message and continued operating normally.

### Transmission Rate

The Vehicle ECU was tested with 100 ms, 500 ms, and 1000 ms transmission periods. Higher transmission frequency produced faster log growth and more frequent Dashboard updates. Lower frequency reduced traffic but reduced responsiveness.

### Node Failure and Diagnostics

The Vehicle ECU was terminated while the Dashboard and Logger remained active. No new CAN messages were received after transmission stopped. The Dashboard used the Speed message as a heartbeat and generated an offline warning after two seconds without a Speed message.

## 6. CAN FD Demonstration

CAN FD was successfully demonstrated using Linux SocketCAN. Classical CAN supports up to 8 bytes of payload, whereas CAN FD supports up to 64 bytes.

The CAN FD application used:

```text
struct canfd_frame
```

and enabled CAN FD support using the `CAN_RAW_FD_FRAMES` socket option. A 20-byte CAN FD frame was successfully transmitted through the virtual CAN interface. Bit Rate Switch was also enabled in the demonstration.

CAN FD is useful when larger payloads or higher data-phase transmission rates are required.

## 7. Conclusion

This project demonstrated that CAN applications can be developed and tested entirely in software using Linux SocketCAN and the Virtual CAN interface. Three independent C-based software ECUs successfully communicated through `vcan0`.

The project demonstrated CAN frame transmission and reception, multi-application traffic observation, CAN ID filtering, traffic logging, unknown message handling, transmission-rate effects, communication-loss detection, and CAN FD communication.

The main learning outcome is that SocketCAN provides a standardized and hardware-independent interface for CAN application development. This allows software development and testing to begin before physical CAN hardware becomes available.
