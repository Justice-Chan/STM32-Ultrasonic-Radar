# Firmware

This directory contains the STM32CubeIDE project for the STM32F411CEU6 target.
Peripheral drivers are implemented directly through memory-mapped registers;
the STM32 HAL is not used.

## Import into STM32CubeIDE

1. Open STM32CubeIDE with a workspace that does not already contain a project
   named `STM32_Supersonic_Artillery`.
2. Choose **File > Import > Existing Projects into Workspace**.
3. Select this `firmware` directory as the project root.
4. Build the Debug configuration.
5. Create a local STM32 C/C++ Application debug configuration for the ST-Link.

The generated `.launch` file is intentionally excluded because it contains a
machine-specific absolute log path. STM32CubeIDE will recreate it locally.

For module ownership, execution contexts, state transitions, and DMA buffer
policy, see [Firmware Architecture](../docs/architecture.md).
