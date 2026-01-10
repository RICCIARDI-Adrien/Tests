# Setup instructions

The ST-Link Virtual Com Port is not wired to chip serial port. In order to enable console output you should use a serial cable and connect it to UART2 pins (PA2/PA3).

# Build instructions

```
# Build the firmware
west build -p -b stm32f4_disco .
```
