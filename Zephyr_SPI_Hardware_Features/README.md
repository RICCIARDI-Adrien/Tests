# Setup instructions

**Note :** These instructions are not needed for NCS versions >= 3.2.0.

You need first to enter the NCS shell before running the following commands, see the build instructions for more details.

```
# Add the SPIM hardware chip select support
west patch gh-fetch -pr 95990 -m zephyr
west patch apply
```

# Build instructions

```
export NCS_VERSION=v3.2.1

# Enter the NCS shell
nrfutil sdk-manager toolchain launch --ncs-version ${NCS_VERSION} --shell
source ~/ncs/${NCS_VERSION}/zephyr/zephyr-env.sh

# Build the firmware
west build -p -b nrf54h20dk/nrf54h20/cpuapp -T ./timer132

# Flash the firmware
west flash
```
