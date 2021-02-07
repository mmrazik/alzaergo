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
2. D-RX (brown wire)
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