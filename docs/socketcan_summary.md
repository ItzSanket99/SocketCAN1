# SocketCAN Summary

## What is SocketCAN?

SocketCAN is the Linux framework that provides CAN communication through the Linux networking subsystem. It allows CAN applications to communicate using standard Linux socket APIs. CAN interfaces such as `can0` or the virtual `vcan0` interface appear as network interfaces to user-space applications.

Applications create a CAN raw socket using `socket(PF_CAN, SOCK_RAW, CAN_RAW)`, bind it to a CAN interface, and then use standard `read()` and `write()` operations to receive and transmit CAN frames.

## Why is CAN treated as a network interface?

CAN is a multi-node message-oriented communication network. Multiple ECUs can transmit and receive messages on the same CAN bus, and messages are identified by CAN identifiers rather than a destination address. Linux therefore integrates CAN into its networking architecture so applications can use a consistent socket-based communication model.

For this project, `vcan0` provides a software-only CAN interface. It behaves like a CAN network interface without requiring a physical CAN controller or transceiver.

## SocketCAN vs Vendor-Specific CAN APIs

Traditional vendor-specific CAN APIs are generally tied to a particular CAN controller, USB adapter, or manufacturer-specific driver. Applications written using such APIs may require significant changes when the hardware platform changes.

SocketCAN provides a standardized Linux interface. Applications communicate with the Linux CAN interface rather than directly depending on a specific CAN hardware vendor.

## Advantages of SocketCAN

The SocketCAN architecture provides several advantages:

* Standard Linux socket programming model.
* Hardware-independent application development.
* Support for multiple applications accessing the same CAN interface.
* Kernel-level CAN message filtering.
* Support for Classical CAN and CAN FD.
* Easy integration with Linux networking and diagnostic tools.
* Software-only testing using virtual CAN interfaces.
* Useful command-line tools such as `candump` and `cansend`.

In this project, SocketCAN enabled the complete CAN application to be developed and tested without physical CAN hardware. The Vehicle ECU, Dashboard ECU, and Logger ECU communicate through the Linux `vcan0` interface.
