# SocketCAN: Software-Only CAN/CAN FD Communication on Linux

A C-based demonstration of Classical CAN and CAN FD communication using Linux SocketCAN and Virtual CAN (`vcan0`), developed without physical CAN hardware.

## Project Overview

The project implements three software ECUs:

* **Vehicle ECU** — Generates and transmits vehicle parameters.
* **Dashboard ECU** — Receives and decodes vehicle information.
* **Logger ECU** — Records all observed CAN traffic to a CSV file.

Additional features include CAN ID filtering, unknown message handling, transmission-rate experiments, ECU failure detection, basic diagnostics, and CAN FD communication.

## Architecture

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

## Requirements

* Linux system
* GCC
* Linux SocketCAN
* `can-utils`
* `iproute2`

Install the required packages:

```bash
sudo apt update
sudo apt install build-essential can-utils iproute2
```

## Setup Virtual CAN

```bash
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
```

Verify:

```bash
ip link show vcan0
```

## Build

From the project root:

```bash
gcc -Wall -Wextra src/vehicle_ecu/vehicle_ecu.c -o vehicle_ecu
gcc -Wall -Wextra src/dashboard_ecu/dashboard_ecu.c -o dashboard_ecu
gcc -Wall -Wextra src/logger_ecu/logger_ecu.c -o logger_ecu
gcc -Wall -Wextra src/can_fd/can_fd_demo.c -o can_fd_demo
```

## Run the Classical CAN System

### Terminal 1 — Vehicle ECU

```bash
./vehicle_ecu
```

### Terminal 2 — Dashboard ECU

```bash
./dashboard_ecu all
```

### Terminal 3 — Logger ECU

```bash
./logger_ecu
```

The Logger stores CAN traffic in:

```text
logs/can_traffic.csv
```

## Dashboard Filtering

Receive all messages:

```bash
./dashboard_ecu all
```

Receive only Speed messages:

```bash
./dashboard_ecu speed
```

Receive only RPM messages:

```bash
./dashboard_ecu rpm
```

## Observe CAN Traffic

```bash
candump vcan0
```

## Send an Unknown Message

```bash
cansend vcan0 200#12345678
```

The Logger records the message while the Dashboard identifies it as an unknown CAN ID.

## CAN FD Demonstration

Configure the interface for CAN FD:

```bash
sudo ip link set vcan0 down
sudo ip link set vcan0 mtu 72
sudo ip link set vcan0 up
```

Run:

```bash
./can_fd_demo
```

Observe:

```bash
candump vcan0
```

After the CAN FD demonstration, the interface can be restored for Classical CAN:

```bash
sudo ip link set vcan0 down
sudo ip link set vcan0 mtu 16
sudo ip link set vcan0 up
```

## CAN Message Definition

| CAN ID  | Signal              | DLC |
| ------- | ------------------- | --: |
| `0x100` | Vehicle Speed       |   2 |
| `0x101` | Engine RPM          |   2 |
| `0x102` | Coolant Temperature |   1 |

## Repository Structure

```text
socketcan-project/
├── src/
│   ├── vehicle_ecu/
│   ├── dashboard_ecu/
│   ├── logger_ecu/
│   └── can_fd/
├── include/
├── docs/
├── logs/
├── tests/
├── README.md
└── .gitignore
```

## Learning Outcomes

The project demonstrates:

* Linux SocketCAN architecture
* Virtual CAN networking
* CAN frame transmission and reception
* CAN message encoding and decoding
* Multiple CAN applications
* Kernel-level CAN filtering
* CAN traffic logging
* Unknown message handling
* Transmission-rate effects
* Node failure detection
* Basic diagnostics
* Classical CAN vs CAN FD
* Software-only CAN development

## Author

Developed as part of an Embedded Systems Honors course assignment.

