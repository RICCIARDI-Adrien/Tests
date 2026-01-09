# Build instructions

```
export NCS_VERSION=v3.2.1

# Enter the NCS shell
nrfutil sdk-manager toolchain launch --ncs-version ${NCS_VERSION} --shell
source ~/ncs/${NCS_VERSION}/zephyr/zephyr-env.sh

# Build the firmware
west build -p -b nrf54h20dk/nrf54h20/cpuapp .
```
