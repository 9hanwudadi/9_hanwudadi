# 软件架构

## 依赖方向

项目的主依赖链只有一个方向：

```text
User -> App -> Driver -> Lib
          |
          +----> OS/RTX51 Tiny
```

- `User/main.c` 只包含 `App.h`，负责调用 `APP_System_Init()`、创建三个 App 任务并删除启动任务。
- `App/` 拥有业务语义：机器人动作模式、步态/动作组合、命令映射和安全策略。
- `Driver/` 把“舵机角度”“PCA9685 PWM”“传感器读数”等硬件能力包装成稳定接口，不决定机器人应该采取什么动作。
- `Lib/` 封装 MCU 寄存器和基础外设，不知道机器人、姿态或舵机编号的业务含义。
- Driver 和 Lib 都不得包含 `App.h`；Lib 不得反向依赖 Driver。

因此，前进/后退/转向、抬腿顺序、插值、姿态切换等逻辑属于 App。PCA9685 寄存器、软件 I2C 时序、PWM 计数换算等逻辑属于 Driver/Lib。App 动作代码不得直接调用 `SI2C_*`，也不得直接写 PCA9685 寄存器。

## `App_System.c` 的窄例外

`App_System.c` 是唯一允许越过 Driver、直接调用部分 Lib 平台初始化接口的 App 文件。它负责一次性的启动编排：开启扩展寄存器访问、配置控制台 GPIO/UART/NVIC 和串口引脚切换，然后调用 Driver 初始化。

这个例外只覆盖平台启动，不代表 App 可以普遍直接使用 Lib。动作、命令和安全模块仍必须通过 Driver 访问硬件；新的平台初始化需求也应集中放在 `App_System.c`，不要散落到其他 App 文件。

## 启动顺序

`main_task` 是 RTX 任务 0，启动流程如下：

```text
main_task
  -> APP_System_Init
       -> EA = 1, EAXSFR
       -> P3.0/P3.1 GPIO
       -> UART1 (Timer1, 115200)
       -> UART1 NVIC and P3.0/P3.1 route
       -> PCA9685_Init
       -> Servo_Init
       -> Bluetooth_Init
       -> Ultrasonic_Init
       -> Battery_Init
       -> Buzzer_Init
       -> Light_Init
  -> create motion/command/safety tasks only when APP_OK
  -> delete task 0
```

PCA9685 初始化内部配置 P3.2/P3.3 软件 I2C 和 50 Hz；App 不单独调用软件 I2C 初始化。`Servo_Init()` 当前会再次调用幂等的 `PCA9685_Init()`。可选外设的 `Init` 当前是无动作占位，成功返回不表示对应实物功能可用，实际操作仍可能返回 `APP_ERR_NOT_READY`。

## 任务与数据流

| RTX 任务 | 编号 | 职责 |
| --- | ---: | --- |
| `task_robot_motion` | 1 | 读取目标模式，选择占位姿态并调用 `Servo_SetPose` |
| `task_command` | 2 | 预留命令接收适配；字节映射由 `APP_Command_OnByte` 完成 |
| `task_safety` | 3 | 读取电池和超声波状态，危险时请求 `STOP` |

当前使用简单的 `volatile` 状态变量。动作模式读写通过短暂关闭全局中断保证原子性；App API 只应从任务上下文调用，不应从 ISR 调用。软件 I2C 与 `F0` 状态共享，PCA9685 调用也应在任务上下文串行执行。

## 错误归属

Driver 只校验参数、执行硬件操作并返回 `int8` 状态；App 决定失败后的业务动作。例如，姿态写入可能在中途遇到 I/O 错误，Driver 返回 `APP_ERR_IO`，动作任务再请求安全停止。高频任务和中断中不要用大量 `printf` 掩盖时序问题。

公共错误码为 `0`、`-1`、`-2`、`-3`、`-4`，详见 [接口总览](interfaces/README.md)。所有可能返回负数的公开接口必须保持 `int8`，不能改成普通 `char`。
