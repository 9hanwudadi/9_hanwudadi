# Driver host behavior tests

From the repository root, run `./tests/driver/run.ps1`. GCC must be on PATH,
or pass `-Compiler C:/path/to/gcc.exe`. Binaries are built in a unique system
temporary directory and removed after the run.

The harness compiles the actual seven Driver sources as strict C90 with
warnings treated as errors and unsigned plain char. Only Config's MCU/type
boundary, GPIO initialization, software I2C, and delay are replaced. The host aliases preserve
C51's 8-bit and 16-bit widths. No host result proves target timing or wiring;
the complete project still needs a Keil C51 build and hardware validation.

The I2C spy records real Driver register writes and models the imported
library's F0 ACK result. Tests catch invalid-input bus access, wrong device
address/register byte order, wrong prescale/clock, omitted wake delay,
wrong servo channel/pulse conversion, masked I/O failures, partial writes
for an invalid pose, fabricated sensor readings, and active placeholder
operations. Initialization tests require port 3, pin mask 0x0C, and open-drain
mode before any I2C write, and reject GPIO failures without bus activity.
Frequency and pulse expectations are hand-derived literals.

The servo fixture assumes the documented neutral calibration defaults:
channels 0 through 7, positive directions, and zero angle offsets. At 50 Hz,
1000/1250/1500/1750/2000 microseconds round to 205/256/307/358/410 PWM counts.
Frequency divisors use the PCA9685's nominal 25 MHz oscillator, independent
of the MCU clock. Actual servo timing also depends on oscillator tolerance
and the quantized frequency divisor.

Init configures P3.2/P3.3 as open-drain before the PCA9685 frequency, without
sending a servo pose. External I2C pull-ups are required. Initialization does
not guarantee that existing outputs are disabled. Calls must be serialized
in task context, and servo users must retain 50 Hz. A failed pose may have
already applied earlier channels. The application owns recovery/safety.
