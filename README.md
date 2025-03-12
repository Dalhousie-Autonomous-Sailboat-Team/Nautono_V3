# Nautono V3 (Windbound)

This is the firmware repository for Windbound (Formerly Nautono V3) - An autonomous sailboat created by Dalhousie's Microtransat Autonomous Sailboat Team (DalMAST).

This revision is based around the STM32H573 microcontroller, and runs FreeRTOS.  This project is intended to be developed outside of the STM32CubeIDE environment.  Instead, STM32CubeMX is used for pin configuration and peripheral initialization, and binaries are generated using the CMake build system.  VSCode with the STM32 and Cortex-Debug extensions is recommended for development.

## Project Structure

- `cmake/`: CubeMX generated CMake files
- `Core/`: CubeMX generated source files - **main.c is in this directory**
- `Drivers/`: External drivers and libraries
- `Middlewares/`: RTOS and other middleware libraries
- `Templates/`: Template files for user source and header files
- **`User/`**: User created source files - **This is where you should put your code**
- `build/`: Temporary Build directory
- CMakelists.txt: Main CMake file - **Add any new source files here**

## Getting Started

**This guide assumes you are using VSCode.**

1. Ensure **Cortex-Debug** and **STM32 VS Code Extension** are installed in VSCode.
2. Ensure **STM32CubeCLT** is installed on your system and added to your PATH.
3. Ensure **STM32CubeMX** is installed on your system.
4. Ensure the **J-Link** drivers are installed on your system and added to your PATH.
5. Open the project folder in VSCode.
6. Run the `CMake: Configure` command from the command palette.
7. Run the `CMake: Build` command from the command palette.
8. Connect your J-Link debugger to the board.
9. Ensure the board is powered on.
10. Run the `CMake: Debug` command from the command palette to start a debug session.

## Debugging

If you get errors relating to unknown commands, ensure that both STM32CubeCLT and J-Link are properly added to your PATH.

If you get errors relating to the build system, make sure you have the STM32CubeCLT installed and added to your PATH.  Ensure you are using the GCC for ARM compiler.

I have not tested this board using an ST-Link debugger.  I highly suggest using J-Link.

## Notes

LED 1 is currently used as a heartbeat indicator.  It will blink every 1000ms.
LED 2 is currently used as a sleep indicator.  It will turn on when the board is in sleep mode.  This signal is also mapped to GPIO4.

The MCU will attempt to go to sleep when there are no tasks to run.  It will wake up when an interrupt is triggered.  This is the first level of low power mode offered by this MCU.

## Task Schedule

| Task            | Period  |
|-----------------|---------|
| Heartbeat Task  | 1000ms  |
| Measure Power   | TBD     |
| Measure Angles  | TBD     |
| Measure GPS     | TBD     |
| Measure Wind    | TBD     |
| Send Data       | TBD     |
