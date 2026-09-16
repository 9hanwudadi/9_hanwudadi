# Servo Driver 接口

对应文件：`Driver/servo.h`、`Driver/servo.c`。模块把八个逻辑舵机 ID 映射到 PCA9685 通道，并负责方向、零位和角度到 PWM 计数的换算。

## 声明

```c
#define SERVO_COUNT 8

int8 Servo_Init(void);
int8 Servo_SetAngle(u8 servo_id, u8 angle_deg);
int8 Servo_SetPose(const u8 *angles, u8 count);
```

三个函数的返回类型必须保持 `int8`，以正确表达负错误码。

## `Servo_Init`

调用 `PCA9685_Init()`，配置 P3.2/P3.3 软件 I2C 并选择 50 Hz。它不会发送姿态。初始化成功返回 `DRIVER_OK`，GPIO/I2C 失败返回 `DRIVER_ERR_IO`。系统启动时 `APP_System_Init()` 已先调用一次 `PCA9685_Init()`，随后再调用 `Servo_Init()`；当前重复初始化是幂等的。

## `Servo_SetAngle`

| 参数 | 有效范围 | 含义 |
| --- | --- | --- |
| `servo_id` | 0..7 | 逻辑舵机编号；当前一一映射到 PCA9685 通道 0..7 |
| `angle_deg` | 0..180 | 目标角度，单位为度；越界不会钳位 |

成功返回 `DRIVER_OK`；编号、角度或应用校准后角度越界返回 `DRIVER_ERR_PARAM`；I2C 写失败返回 `DRIVER_ERR_IO`。调用前必须先成功初始化，并保持 PCA9685 为 50 Hz。

## `Servo_SetPose`

`angles` 必须指向至少八个 `u8` 角度，`count` 必须严格等于 `SERVO_COUNT`（8），每个角度都必须在 0..180。空指针、数量不是 8 或任一角度无效时返回 `DRIVER_ERR_PARAM`。

函数会先验证全部八个角度，再开始写通道，所以参数错误不会造成部分更新。但 I2C 错误可能发生在写入中途，已写入的前几个通道不会自动回滚；调用方必须把非零返回当成姿态可能不完整，并执行安全策略。

## 最小示例

```c
#include "servo.h"

static const u8 neutral_pose[SERVO_COUNT] = {
    90, 90, 90, 90, 90, 90, 90, 90
};

int8 ApplyNeutralPose(void)
{
    int8 status;

    status = Servo_Init();
    if (status != DRIVER_OK) {
        return status;
    }
    return Servo_SetPose(neutral_pose, SERVO_COUNT);
}
```

App 层通常不应重复初始化，而是在 `APP_System_Init()` 成功后调用舵机操作。高层动作应使用 `Servo_SetPose`/`Servo_SetAngle`，不能直接写 PCA9685 寄存器或软件 I2C。

## 校准与实物限制

当前 `servo.c` 的通道 `{0..7}`、方向全 `+1`、零偏全 `0`、0..180° 到 1000..2000 us 的换算都是中性占位值，不是实物标定结果。接通舵机前必须逐路确认通道对应关节、安装方向、中立角、机械极限和安全脉宽，并确认舵机电源电流、共地及 P3.2/P3.3 外部上拉。校准结果同步登记到 [硬件资源表](../hardware-resources.md)。
