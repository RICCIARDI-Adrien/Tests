# Setup instructions

You need first to enter the NCS shell before running the following commands, see the build instructions for more details.

```
# Add the SPIM hardware chip select support
west patch gh-fetch -pr 95990 -m zephyr
west patch apply
```

# Build instructions

```
# Enter the NCS shell
nrfutil sdk-manager toolchain launch --ncs-version v3.2.0-preview2 --shell
source ~/ncs/v3.2.0-preview2/zephyr/zephyr-env.sh

# Build the firmware
west build --sysbuild -p -b nrf54h20dk/nrf54h20/cpuapp .

# Flash the firmware
west flash
```
