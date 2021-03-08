# AlzaErgo Table ET1 NewGen
This is my attempt to reverse engineer [AlzaErgo Table ET1 NewGen](https://www.alza.cz/alzaergo-table-et1-newgen-white-d5647311.htm) standing desk and add wi-fi functionality using something like Wemos D1/ESP8266.

The work is in progress.

# Components
### Control Panel
Controls height of the desk, show the current height on display, has memory for 3 positions, you can set min/max height, etc.

![Control Panel](images/control_panel.png)

### Control Box
Contains the PowerSupply Unit, controls the motors and, gets commands from the Control Panel.

![Control Box](images/control_box.png)

### Connection
The control box has a RJ-45 port. It seems like all 8 pins are being used but the provided Control Panel is only using 5 of them and does not wire the rest at all.

![Control Box](images/control_box_pins.png)


The wiring of the RJ45 port is as follows:
<img align="right" src="images/rj45.png">
1. D-TX (green wire)
2. D-RX (brown wire; control panel transmits here)
3. unused
4. KEY-1 (white wire)
5. G (red wire)
6. unused
7. unused
8. +5V (yellow wire)

<br style="clear:both" />


On the Control Panel side it looks like this:
![Control Box](images/control_panel_pinout.png)


## Protocol
Seems like the protocol is using the following structure: <br/>
``[command byte] [byte1] [byte2] [...] [byteN] [checksum]``

The checksum is calculated as (where `&` is bitwise `AND`): <br/>
```(byte1 + byte2 + ... + [byteN]) & 0xff```

### Panel messages:
Few random observations
```
A5 0 0 1 1    Idle
A5 0 20 1 21  Move up
A5 0 40 1 41  Move down
A5 0 60 1 61  UP and Down (used to reset)


A5 0 1 1 2    M button
A5 0 2 1 3    memory 1
A5 0 4 1 5    memory 2
A5 0 8 1 9    memory 3
A5 0 10 1 11  T button
A5 0 11 1 12  M+T (to get into settings)
```
### Board messages
#### Table height
Command: `0x5A [byte1] [byte2] [byte3] [byte4] [checksum]` <br/>

`byte1`, `byte2`, `byte3` are binary encoded decimals for 7 segment display. Each bit in the byte corresponds to one segment. For example 8 is represented by `0b01111111` (or `0b11111111`; the topmost bit is not relevant):
```
   __0_
  |    |
5 |    | 1
  |__6_|
  |    |
4 |    | 2
  |____|
     3
```

For example 6 can be represented as 0b01111101 (or 0b11111101)
```
   __0_
  |
5 |
  |__6_
  |    |
4 |    | 2
  |____|
     3
```

The top most bit of the middle byte is signaling the decimal point.
`0x06 0xbf 0x06` translates to `10.1` on display while `0x06 0x3f 0x06` translates to `101` on the display.

The full list of digits:
| Digit | Hex                | binary                       |
|---    | ---                | ---                          |
| 0     | `0x3f` or `0xbf`   | `0b00111111` or `0b10111111` |
| 1     | `0x06` or `0x86`   | `0b00000110` or `0b10000110` |
| 2     | `0x5b` or `0xdb`   | `0b01011011` or `0b11011011` |
| 3     | `0x4f` or `0xcf`   | `0b01001111` or `0b11001111` |
| 4     | `0x66` or `0xe6`   | `0b01100110` or `0b11100110` |
| 5     | `0x6d` or `0xed`   | `0b01101101` or `0b11101101` |
| 6     | `0x7d` or `0xfd`   | `0b01111101` or `0b11111101` |
| 7     | `0x07` or `0x87`   | `0b00000111` or `0b10000111` |
| 8     | `0x7f` or `0xff`   | `0b01111111` or `0b11111111` |
| 9     | `0x6f` or `0xef`   | `0b01101111` or `0b11101111` |

The byte sequence of `5A 07 FD 6D 10 81 ` therefore corresponds to table height `76.5` cm (byte `07` corresponds to `7`, byte `FD` corresponds to `6` and byte `6d` corresponds to `5`).