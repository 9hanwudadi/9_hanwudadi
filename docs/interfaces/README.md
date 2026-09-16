# 接口总览

本目录描述当前头文件里的公开接口。详细舵机接口见 [servo.md](servo.md)，动作接口见 [robot-motion.md](robot-motion.md)。调用者以头文件为唯一编译契约；接口变化必须同步更新文档。

## 公共返回码

所有可能返回负错误码的接口都使用 `int8`（`signed char`），不要改成普通 `char`。

| 宏 | 值 | 含义 |
| --- | ---: | --- |
| `APP_OK` / `DRIVER_OK` | 0 | 成功 |
| `APP_ERR_PARAM` / `DRIVER_ERR_PARAM` | -1 | 参数、指针、编号或范围错误 |
| `APP_ERR_NOT_READY` / `DRIVER_ERR_NOT_READY` | -2 | 第一版尚未实现或硬件资源未就绪 |
| `APP_ERR_IO` / `DRIVER_ERR_IO` | -3 | GPIO、I2C、UART 或设备通信失败 |
| `APP_ERR_BUSY` / `DRIVER_ERR_BUSY` | -4 | 资源忙；当前接口预留此码 |

## 初始化顺序

应用只调用 `APP_System_Init()`。它按当前实现执行：CPU/扩展寄存器访问 -> P3.0/P3.1 GPIO -> UART1/Timer1/115200 -> UART1 NVIC/引脚切换 -> `PCA9685_Init()` -> `Servo_Init()` -> `Bluetooth_Init()` -> `Ultrasonic_Init()` -> `Battery_Init()` -> `Buzzer_Init()` -> `Light_Init()`。

`PCA9685_Init()` 内部配置 P3.2/P3.3 软件 I2C 和 50 Hz；不要从 App 另行初始化 I2C。`Servo_Init()` 当前会再次进行同样的 PCA9685 初始化。系统返回 `APP_OK` 后，`main_task` 才创建动作、命令和安全任务。

## `App/App.h`

```c
int8 APP_System_Init(void);
int8 APP_Robot_SetMotion(RobotMotionMode mode);
void APP_Command_OnByte(u8 value);
int8 APP_Safety_Check(void);

void task_robot_motion(void);
void task_command(void);
void task_safety(void);
```

- `APP_System_Init`：按上述顺序初始化；核心 GPIO/UART/PCA9685/Servo 任一失败立即返回对应错误。可选外设 `Init` 的结果仅内部保存；当前无动作初始化返回成功不表示操作接口可用。
- `APP_Robot_SetMotion`：见 [robot-motion.md](robot-motion.md)。
- `APP_Command_OnByte`：映射 `'0'` STOP、`'1'` STAND、`'2'` CROUCH、`'w'` FORWARD、`'s'` BACKWARD、`'a'` TURN_LEFT、`'d'` TURN_RIGHT、`'h'` GREET；未知字节被忽略。它没有返回值，当前也不会把动作返回码传给调用者。
- `APP_Safety_Check`：始终尝试读取电池和超声波；任一已就绪传感器确认危险时请求 STOP。若传感器未就绪，优先返回电池错误，否则返回超声波状态。当前阈值 6.0 V 和 15.0 cm 都需实物校准。
- 三个 `task_*` 是 RTX 任务入口，由 `main_task` 创建，不是普通业务调用 API。`_task_` 属性只写在任务函数定义上，不写在头文件原型中，否则 Keil C51 会报 C141。

## `Driver/pca9685.h`

```c
int8 PCA9685_Init(void);
int8 PCA9685_SetFrequency(u16 frequency_hz);
int8 PCA9685_SetPwm(u8 channel, u16 on_count, u16 off_count);
```

- `PCA9685_Init`：配置 P3.2/P3.3 开漏并选择 50 Hz，不发送任何舵机姿态。外部上拉必需。
- `PCA9685_SetFrequency`：范围 40..1000 Hz；超出返回 `DRIVER_ERR_PARAM`。
- `PCA9685_SetPwm`：`channel` 为 0..15，`on_count`/`off_count` 为 0..4095；不支持 full-on/full-off 标志位。调用前必须初始化。
- 软件 I2C 和 `F0` 被共享，只能从任务上下文串行调用；I2C ACK 失败返回 `DRIVER_ERR_IO`。

## `Driver/servo.h`

```c
#define SERVO_COUNT 8
int8 Servo_Init(void);
int8 Servo_SetAngle(u8 servo_id, u8 angle_deg);
int8 Servo_SetPose(const u8 *angles, u8 count);
```

参数、部分写入风险和最小示例见 [servo.md](servo.md)。

## 其他第一版 Driver

```c
int8 Bluetooth_Init(void);
int8 Bluetooth_SendByte(u8 value);

int8 Ultrasonic_Init(void);
int8 Ultrasonic_GetDistance(float *distance_cm);

int8 Battery_Init(void);
int8 Battery_GetVoltage(float *voltage_v);

int8 Buzzer_Init(void);
int8 Buzzer_Beep(u16 frequency_hz, u16 duration_ms);
void Buzzer_Stop(void);

int8 Light_Init(void);
int8 Light_Set(u8 light_id, u8 enabled);
```

这些 `Init` 当前是无动作占位并返回 `DRIVER_OK`。`Bluetooth_SendByte`、有效指针下的 `Ultrasonic_GetDistance`/`Battery_GetVoltage`、`Buzzer_Beep` 和 `Light_Set` 均返回 `DRIVER_ERR_NOT_READY`，因为引脚或外设资源尚未确认；两个 Get 函数遇到空指针返回 `DRIVER_ERR_PARAM` 且未就绪时不修改输出。`Buzzer_Stop` 当前是安全空操作。不得用 `Init` 的成功返回推断硬件已可用。

## 最小系统调用

正常工程由 `User/main.c` 自动初始化和创建任务。若仅说明调用关系，最小顺序如下：

```c
if (APP_System_Init() == APP_OK) {
    (void)APP_Robot_SetMotion(ROBOT_MOTION_STAND);
}
```

实际 RTX 工程不要在 `main_task` 中持续执行动作；应让已创建的动作任务处理目标模式。
