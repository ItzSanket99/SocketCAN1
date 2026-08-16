# CAN Message Definition

## CAN Frame Structure

A Classical CAN data frame used by this project contains:

```text
┌──────────────┬─────┬──────────────────────┐
│ CAN ID       │ DLC │ Data Payload         │
├──────────────┼─────┼──────────────────────┤
│ 11-bit ID    │ 0-8 │ 0-8 data bytes       │
└──────────────┴─────┴──────────────────────┘
```

The project uses standard 11-bit CAN identifiers.

## Message Mapping

| CAN ID  | Signal              | DLC | Encoding   | Range        |
| ------- | ------------------- | --: | ---------- | ------------ |
| `0x100` | Vehicle Speed       |   2 | Value × 10 | 0–120 km/h   |
| `0x101` | Engine RPM          |   2 | Integer    | 800–5000 rpm |
| `0x102` | Coolant Temperature |   1 | Integer    | 20–120 °C    |

## Vehicle Speed

Speed is encoded with a resolution of 0.1 km/h.

```text
Encoded value = Speed × 10
```

For example:

```text
65 km/h × 10 = 650
650 decimal = 0x028A
```

The payload is:

```text
02 8A
```

## Engine RPM

RPM is stored directly as a 16-bit unsigned value.

For example:

```text
2450 rpm = 0x0992
```

Payload:

```text
09 92
```

## Coolant Temperature

Temperature is stored directly as an unsigned 8-bit value.

For example:

```text
88 °C = 0x58
```

Payload:

```text
58
```

## Standard vs Extended Identifier

The project uses standard CAN identifiers, which contain 11 identifier bits. CAN also supports extended identifiers containing 29 bits.

Standard identifiers were selected because they are sufficient for the small vehicle network implemented in this assignment and are easy to filter using SocketCAN.
