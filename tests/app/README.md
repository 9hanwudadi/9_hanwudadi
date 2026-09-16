# App host behavior tests

Run `./tests/app/run.ps1` from the repository root. GCC must be on PATH;
`-Compiler C:/path/to/gcc.exe` selects another installation. The runner
uses strict C90, warnings as errors, and unsigned plain char. Generated
source copies and the executable live in a unique system temporary folder
and are removed on exit.

The runner transforms only the nonstandard C51 `_task_ <ID>` attributes
in temporary copies of the real App sources, App.h, and User/main.c. It
does not replace any App function or add a production test hook. Driver
headers are real. Narrow Driver, MCU/core-library, and RTX stubs capture
effects at the hardware boundary. MCU aliases preserve the 8-bit signed
status type and 16-bit `u16`. The host `u32` need only represent the UART
baud value; target ABI, memory spaces, timing, and task scheduling still
require Keil C51 and hardware validation.

The harness includes the transformed sources in one translation unit so
it can inspect the required file-static requested-motion enum. This is
needed to distinguish five unsupported requests with the same safe output.
It also checks complete eight-servo output poses through Servo_SetPose.
The RTX wait stub uses setjmp/longjmp to stop infinite tasks at their
normal yield points; the task bodies themselves are unchanged.

Coverage catches skipped/reordered initialization, uninitialized UART
settings, swallowed required errors, optional init failures blocking boot,
invalid enum mutation, unsupported state loss, wrong/incomplete poses,
pose-error recovery, missing command mappings, unknown-byte mutation,
false safety emergencies from unavailable sensors, threshold boundaries,
unexamined second sensors, missing task yields, and incorrect startup
task creation/deletion. All 256 command bytes are checked. Sensor stubs
deliberately write unsafe values when returning errors to verify gating.

The fixture uses approved calibration placeholders: STOP is eight 90-degree
angles; STAND alternates 90/60; CROUCH alternates 90/120. IDs are tentatively
four hip/knee pairs. STOP requests a neutral pose; it does not disable PWM
or power. Confirm joint order, directions, mechanical limits, and zeros
before hardware use. A failed pose requests STOP for the next cycle and
retains the Driver error in the motion diagnostic status; bus failure can
still prevent the safe pose from being applied.

APP_Safety_Check samples both sensors, stops on a confirmed voltage below
6.0 V or distance below 15.0 cm, and returns the first read error (battery
first). Exact thresholds are allowed. These are initial calibration values.
Optional init results are diagnostic only: inert Init success does not
establish hardware readiness. Only successful sensor reads authorize value
use. The current command task only yields because no Driver receive API
exists; a future task-context adapter should call APP_Command_OnByte.

Only App_System uses GPIO/UART/NVIC/Switch directly for core initialization.
UART1 uses P3.0/P3.1, Timer1, 115200 baud, and priority 1. RTX keeps Timer0.
PCA9685 initializes its own I2C pins. The explicit Servo_Init call currently
repeats PCA9685_Init inside the Driver; the repetition is idempotent and
does not send a pose. Task delays are expressed in RTX ticks (motion 2,
command 1, safety 10), not assumed milliseconds. App APIs run in task
context; future ISR receive code must defer App calls to a task.
