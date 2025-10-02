# Build instructions

```
# Enter the NCS shell
nrfutil sdk-manager toolchain launch --ncs-version v3.1.0 --shell
source ~/ncs/v3.1.0/zephyr/zephyr-env.sh

# Build the firmware
west build --sysbuild -p -b nrf54h20dk/nrf54h20/cpuapp .

# Flash the firmware
west flash
```
