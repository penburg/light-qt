# lights-qt
**A home automation program **

This is a home automation program designed to be deployed to a raspberry pi or similar device with gpio’s.

## Warranty
NONE

## Capabilities
- GPIO output controls ie. on / off
- Grouping of GPIO controls
- Thermal sensor reading
- Temperature readings from the US National Weather Service’s API
- Timer events.
- GPIO input event
- Virtual Thermostat
- Grouping GPIO devices to control an evaporative cooler
- Sunrise / sunset timers
- Logging thermal alerts
- Interactive console

## Configuring
Settings can be changed and devices can be added from the interactive console. Removing or changing a device parameters isn’t implemented you will have to manually edit the settings file created by qt. On Linux ~/.config/Enburg Dev/Lights-QT.conf

### Thermal Sensors
Must be expressed on Linux’s sysfs to be read ie. < br/>
/sys/class/thermal/thermal_zone0/temp < br/>
Might be a good place to read the CPU temprature. < br/>
While /sys/bus/w1/devices/28-0000000xxxxx/temperature would be a w1 ds18b20 device

### GPIO Outputs
I recommend using a quality relay board

### GPIO Inputs
I use a 3.3v Optocoupler Isolation Board to isolate my raspberry pi inputs

## Donations
While the software is free if you wish to make a donation to developers coffee fund we prefer the USDC stable coin on Noble (noble1qpnk978h9hklhf6x2gkd2tstj6plmp9qlqzjdm) or a low fee EVM network (0x3dD87612d064EA12ba6bC49DB5BA6712B90442b3) like Base, Arbitrum, Avalanche, Optimism, & Polygon

