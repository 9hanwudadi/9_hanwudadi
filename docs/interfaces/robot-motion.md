# Robot Motion 接口

对应文件：`App/App.h`、`App/APP_RobotMotion.c`。模块保存目标动作模式，由 RTX 动作任务把已支持模式转换成八舵机姿态。步态和动作组合必须留在 App 层。

## 声明与模式

```c
typedef enum {
    ROBOT_MOTION_STOP = 0,
    ROBOT_MOTION_STAND,
    ROBOT_MOTION_CROUCH,
    ROBOT_MOTION_FORWARD,
    ROBOT_MOTION_BACKWARD,
    ROBOT_MOTION_TURN_LEFT,
    ROBOT_MOTION_TURN_RIGHT,
    ROBOT_MOTION_GREET
} RobotMotionMode;

int8 APP_Robot_SetMotion(RobotMotionMode mode);
```

| 模式 | 当前返回 | 动作任务实际输出 |
| --- | --- | --- |
| `ROBOT_MOTION_STOP` | `APP_OK` | `{90,90,90,90,90,90,90,90}` 中立占位姿态 |
| `ROBOT_MOTION_STAND` | `APP_OK` | `{90,60,90,60,90,60,90,60}` 站立占位姿态 |
| `ROBOT_MOTION_CROUCH` | `APP_OK` | `{90,120,90,120,90,120,90,120}` 趴下占位姿态 |
| `ROBOT_MOTION_FORWARD` | `APP_ERR_NOT_READY` | 暂时输出 STOP 占位姿态 |
| `ROBOT_MOTION_BACKWARD` | `APP_ERR_NOT_READY` | 暂时输出 STOP 占位姿态 |
| `ROBOT_MOTION_TURN_LEFT` | `APP_ERR_NOT_READY` | 暂时输出 STOP 占位姿态 |
| `ROBOT_MOTION_TURN_RIGHT` | `APP_ERR_NOT_READY` | 暂时输出 STOP 占位姿态 |
| `ROBOT_MOTION_GREET` | `APP_ERR_NOT_READY` | 暂时输出 STOP 占位姿态 |

枚举范围之外返回 `APP_ERR_PARAM`，且不更新目标模式。所有返回值使用 `int8`。

## 不支持动作的精确语义

对 FORWARD 及之后的合法枚举值，函数会先保存请求的目标模式，再返回 `APP_ERR_NOT_READY`。动作任务看到这些尚未实现的模式时会持续使用 STOP 占位表，不会执行步态。它们不是静默成功，也不是立即改写为 STOP 枚举；若之后舵机写入失败，动作任务才会把目标模式请求为 `ROBOT_MOTION_STOP`。

`ROBOT_MOTION_STOP` 也不是断电或关闭 PWM：它调用 `Servo_SetPose` 输出八路 90° 占位姿态。不要把它当作经实物验证的急停。真正的断电、释放扭矩或安全姿态需要在硬件方案确认后另行设计。

## 最小示例

```c
#include "App.h"

int8 RequestStand(void)
{
    return APP_Robot_SetMotion(ROBOT_MOTION_STAND);
}
```

检查尚未实现的动作时必须处理返回码：

```c
int8 status;

status = APP_Robot_SetMotion(ROBOT_MOTION_FORWARD);
if (status == APP_ERR_NOT_READY) {
    /* The first version keeps outputting the STOP placeholder pose. */
}
```

只能在 `APP_System_Init()` 成功且动作任务已创建后从任务上下文请求动作；不要从 ISR 调用 App API。动作请求只改变目标状态，实际八路写入由 `task_robot_motion` 周期执行。

## 占位姿态与后续开发

三个数组都只是代码骨架，没有经过关节映射、方向、零位、机械极限、负载或供电验证。实物校准前不要直接执行。实现新步态时，在 App 层组织时间序列、插值和状态机，并只调用 Servo Driver；不要在动作文件中写 PCA9685 寄存器或 `SI2C_*`。接口若需变化，按根目录 README 的协调流程先通知所有调用方。
