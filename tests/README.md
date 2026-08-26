# Host-native tests

These tests compile the hand-written firmware modules for the development
computer. Hardware and RTOS dependencies are replaced by deterministic fakes,
so they do not require the STM32 toolchain or a connected board.

From the repository root, run:

```powershell
cmake --preset Host-Tests
cmake --build --preset Host-Tests
ctest --preset Host-Tests
```

On Windows, CMake uses the installed Visual Studio toolchain. On Linux and
macOS, it uses the default host C compiler.

The suite covers:

- control arbitration, timeouts, sail direction, and rudder output;
- PID initialization, state, clamping, and update terms;
- XBee, Raspberry Pi, and wind-vane protocol parsing;
- shared application-state freshness and mutex behavior;
- encoder mux selection, state storage, and a simulated sensor read cycle;
- board I2C/UART wrappers and completion callbacks;
- initialization, heartbeat, and sleep task behavior;
- firmware assertion breakpoints; and
- an integration path from parsed commands through shared state to actuator
  decisions.

The tests intentionally characterize current firmware behavior. Safety and
mode-control changes should update or extend these expectations in the same
commit as the production change.

CubeMX-generated sources are validated by the ARM firmware build rather than
host unit tests. `command_dispatch.c` currently contains no active
implementation, and physical PWM, wiring, radio, and sensor behavior still
require board or hardware-in-the-loop testing.
