# 硬件资源登记

本表只记录当前代码可以确认的资源。任何空缺都写成 `UNASSIGNED`，表示尚未分配，不表示“可随意使用”。新占用或变更必须先由牛迈协调，再在代码和本表中同一 PR 更新。

## MCU 与通信资源

| 资源 | 当前分配 | 状态/依据 | 实物要求 |
| --- | --- | --- | --- |
| MCU 主频 | 24 MHz (`MAIN_Fosc = 24000000L`) | CONFIRMED | 烧录和时钟配置必须与 Keil 工程一致 |
| P3.0 | UART1 RX | CONFIRMED | `UART1_SW_P30_P31`；下载/外接模块复用冲突需现场确认 |
| P3.1 | UART1 TX | CONFIRMED | `UART1_SW_P30_P31`；电平和接线需现场确认 |
| UART1 | 115200, 8-bit，接收使能 | CONFIRMED | 当前作为核心控制台配置 |
| UART1 中断 / NVIC | 已启用，`Priority_1` | CONFIRMED | `NVIC_UART1_Init(ENABLE, Priority_1)`；改优先级前须评估 RTX 和其他中断 |
| Timer1 | UART1 波特率发生器 | CONFIRMED | 不得再分配给舵机或其他周期任务 |
| P3.2 | PCA9685 软件 I2C SCL | CONFIRMED | 开漏输出，必须有合适的外部上拉电阻 |
| P3.3 | PCA9685 软件 I2C SDA | CONFIRMED | 开漏输出，必须有合适的外部上拉电阻 |
| 软件 I2C | PCA9685，总线状态使用 `F0` | CONFIRMED | 任务上下文串行调用；地址/电平需按实物核对 |
| Timer0 | RTX51 Tiny 系统时钟/软件定时 | CONFIRMED | `Conf_tny.A51` 明确使用 8051 Timer0，`INT_CLOCK=10000`，不得重分配 |
| RTX 时间片 | 5 个硬件 Timer tick | CONFIRMED | `TIMESHARING=5` |
| Timer2 | `UNASSIGNED` | UNASSIGNED | 分配前先确认与 UART/PWM 方案无冲突 |
| Timer3 | `UNASSIGNED` | UNASSIGNED | 分配前登记 |
| Timer4 | `UNASSIGNED` | UNASSIGNED | 分配前登记 |
| UART2 | `UNASSIGNED` | UNASSIGNED | 蓝牙尚未绑定串口 |
| UART3 | `UNASSIGNED` | UNASSIGNED | 分配前登记 |
| UART4 | `UNASSIGNED` | UNASSIGNED | 分配前登记 |
| ADC 通道/引脚 | `UNASSIGNED` | UNASSIGNED | 电池分压比和安全量程必须实测 |
| 超声波 Trigger/Echo | `UNASSIGNED` | UNASSIGNED | 引脚、电平和计时资源均待确认 |
| 蜂鸣器引脚/Timer/PWM | `UNASSIGNED` | UNASSIGNED | 有源/无源类型和有效电平待确认 |
| 灯 ID/引脚 | `UNASSIGNED` | UNASSIGNED | 数量及有效电平待确认 |
| 蓝牙串口/控制引脚 | `UNASSIGNED` | UNASSIGNED | 不得默认复用当前控制台 UART1 |

## PCA9685 与八路舵机

PCA9685 当前使用内部 25 MHz 假设值，将频率设为 50 Hz。代码中的舵机 ID 到通道映射只是占位映射：

| 舵机 ID | PCA9685 通道 | 机械关节 | 方向/零位/极限 |
| ---: | ---: | --- | --- |
| 0 | 0 | `UNASSIGNED` | `UNASSIGNED`（代码暂用方向 +1、零偏 0、0–180°） |
| 1 | 1 | `UNASSIGNED` | `UNASSIGNED`（同上） |
| 2 | 2 | `UNASSIGNED` | `UNASSIGNED`（同上） |
| 3 | 3 | `UNASSIGNED` | `UNASSIGNED`（同上） |
| 4 | 4 | `UNASSIGNED` | `UNASSIGNED`（同上） |
| 5 | 5 | `UNASSIGNED` | `UNASSIGNED`（同上） |
| 6 | 6 | `UNASSIGNED` | `UNASSIGNED`（同上） |
| 7 | 7 | `UNASSIGNED` | `UNASSIGNED`（同上） |
| 备用通道 8..15 | 8..15 | `UNASSIGNED` | UNASSIGNED |

禁止在未校准实物时直接使用占位姿态做大范围动作。至少逐路确认机械关节、正反方向、中立角、可用最小/最大角、脉宽范围、电源容量和共地；修改 `servo.c` 校准表后同步更新本表。P3.2/P3.3 的外部上拉不能省略，阻值应结合总线电压、线长和器件数量由硬件成员确认。

## 变更登记最小信息

申请新资源时，在 PR 中写清资源名、用途、初始化位置、占用时段、是否与现有中断/总线共享，以及实物验证结果。资源冲突未解决前保持 `UNASSIGNED`，不要仅凭 Lib 中存在某个驱动函数就宣称资源已分配。
