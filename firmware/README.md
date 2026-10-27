# Firmware environment

No firmware has been implemented or flashed.

Proposed environment: Arduino C++ using an explicitly pinned RP2040 core and SparkFun Qwiic Twist library. Select versions after confirming hardware-I2C and USB-serial support on the XIAO RP2040.

The first executable should only exercise the Twist, RGB LED, and USB serial. Sony and camera outputs remain disabled.

Repository setup does not install a toolchain or flash hardware. Next: inspect local tooling, select versions, and build the serial-only diagnostic.
