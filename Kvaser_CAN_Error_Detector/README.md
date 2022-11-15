# Kvaser CAN Error Detector

This software uses Kvaser CANlib API to report received frames errors.  
It is built with GCC under Cygwin on Windows.

## CANlib installation

* Download the [CANlib setup](https://www.kvaser.com/developer/canlib-sdk) for Windows.
* Install it to the default location `C:\\Program Files (x86)\\Kvaser\\Canlib`.

## Cygwin installation

* Download [Cygwin](https://www.cygwin.com/setup-x86_64.exe).
* Install it to the default location and select the `make` and `gcc-core` packages.

## Build the software

* Clone the repository.
* Enter the repository and type `make`.
