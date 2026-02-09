# Setup instructions

## Linux development computer

To be able to flash the firmware with `west flash`, create an `udev` rule with the following content :
```
KERNEL=="hidraw*", SUBSYSTEM=="hidraw", ATTRS{idVendor}=="03eb", ATTRS{idProduct}=="2169", MODE="0666", GROUP="plugdev"
```

## Board connections

The test has been done with a Cirque `TM035035-2024-003` trackpad converted to I2C (by unpopulating `R1`) connected to a Microchip SAMD21 Xplained Pro board.

Connections :
* SAMD21 board PWR connector `GND` : trackpad `GND`
* SAMD21 board PWR connector `VCC` : trackpad `VDD`
* SAMD21 board EXT1 connector `PA06` : trackpad DR (data ready)
* SAMD21 board EXT1 connector `PA08` : trackpad I2C SDA
* SAMD21 board EXT1 connector `PA09` : trackpad I2C SCL

# Build instructions

```
# Build the firmware
west build -p -b samd21_xpro .
```
