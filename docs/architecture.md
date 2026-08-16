# SocketCAN System Architecture

## System Overview

The system consists of three software ECUs communicating through a Linux virtual CAN interface.

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
       ┌─────────────┐ ┌─────────────┐ ┌─────────────┐
       │ Vehicle ECU │ │ Dashboard   │ │ Logger ECU  │
       │    (C)      │ │ ECU (C)     │ │    (C)      │
       └─────────────┘ └─────────────┘ └─────────────┘
              │               │               │
              │ write()       │ read()        │ read()
              │               │               │
              └──────── CAN frames ───────────┘
```

## Vehicle ECU

The Vehicle ECU acts as the transmitter. It generates realistic vehicle parameters and periodically transmits three CAN messages:

* Vehicle Speed — CAN ID `0x100`
* Engine RPM — CAN ID `0x101`
* Coolant Temperature — CAN ID `0x102`

The values are encoded into CAN payloads and transmitted using a SocketCAN raw socket.

## Dashboard ECU

The Dashboard ECU receives CAN messages from `vcan0`. It decodes the payload according to the defined CAN message format and displays the current vehicle information.

It also supports CAN ID filtering for:

* All messages
* Speed messages only
* RPM messages only

A timeout mechanism monitors the Speed message as a heartbeat. If no Speed message is received for two seconds, the Dashboard reports that the Vehicle ECU is offline.

## Logger ECU

The Logger ECU receives CAN traffic independently of the Dashboard. Each received frame is stored in a CSV file containing:

* Timestamp
* CAN identifier
* Payload length
* Payload data

Because CAN messages are broadcast on the network, multiple applications can observe the same traffic simultaneously.

## Virtual CAN Network

The Linux `vcan0` interface provides the software-only CAN network. No physical CAN controller, transceiver, or CAN cable is required.

This architecture allows the CAN applications to be developed and tested before physical hardware becomes available.
