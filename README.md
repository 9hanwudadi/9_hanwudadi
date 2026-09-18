# STC8H8K64U 四足机器人模板

这是一个面向八人学生团队的 Keil C51/RTX51 Tiny 四足机器人协作骨架。当前目标是让各模块在稳定接口下并行开发，而不是提供已经标定、可以直接行走的成品固件。

功能边界参考 [AffiniaWu/Quadruped_Robot](https://github.com/AffiniaWu/Quadruped_Robot)。本模板没有复制该项目的高层 GPL 动作代码；导入的厂商来源文件保留原有文件头。公开发布前，团队仍须逐项确认所有导入文件及依赖的再分发许可。本说明不构成法律保证，也不替代许可核查。

## 用 Keil 打开和构建

1. 用 Keil uVision 打开仓库根目录的 `stc8h8k64u.uvproj`。
2. 选择工程中已有的 `Target 1`，不要新建或改名 Target。
3. 执行 **Build Target**。
4. 构建完成后检查 `Objects/stc8h8k64u.hex`。

工程配置使用 STC8H8K64U、24 MHz、RTX51 Tiny；编译包含路径为 `./App;./Driver;./Lib;./User`。`Objects/` 和 `Listings/` 是本机构建目录，不是源码。

## 目录职责

| 目录/文件 | 职责 |
| --- | --- |
| `User/main.c` | 只通过 `App.h` 启动系统和 RTX 任务 |
| `App/` | 动作、命令、安全策略与系统编排；步态和动作组合写在这里 |
| `Driver/` | PCA9685、舵机及外围设备驱动；向 App 报告明确状态 |
| `Lib/` | STC 底层库、寄存器封装、UART、Timer、软件 I2C 等 |
| `OS/` | RTX51 Tiny 配置与库 |
| `docs/` | 架构、资源登记和接口契约 |
| `docs/hardware/` | PCA9685、JDY-33、LM2596、LU-ASR01 等硬件参考资料及索引 |
| `tests/`、`tools/` | 主机侧检查和仓库验证脚本，不加入 Keil Target |

完整依赖规则见 [docs/architecture.md](docs/architecture.md)，已确认和待分配的硬件资源见 [docs/hardware-resources.md](docs/hardware-resources.md)，硬件原始资料见 [docs/hardware/README.md](docs/hardware/README.md)，接口入口见 [docs/interfaces/README.md](docs/interfaces/README.md)。

## Git 协作流程

团队仓库为 [9hanwudadi/9_hanwudadi](https://github.com/9hanwudadi/9_hanwudadi)。功能开发从 `develop` 建立个人分支：

```bash
git clone https://github.com/9hanwudadi/9_hanwudadi.git
cd 9_hanwudadi
git switch develop
git switch -c feature/<模块>-<简短目的>
```

每次提交只包含一个清晰目的。提交前检查差异、运行对应模块测试和仓库验证器：

```bash
git status --short
git diff --check
powershell -ExecutionPolicy Bypass -File tools/validate-template.ps1 -Root .
git add <本次修改的文件>
git commit -m "feat: describe the module change"
```

然后把功能分支推到团队仓库并发起 PR。PR 说明至少写清：改了什么、怎样验证、占用了哪些硬件资源、接口是否变化、仍有哪些限制。由牛迈负责接口协调、资源冲突检查和合并；不要直接把未经单测的代码复制进主分支。

### 修改接口时

已经发布给其他模块的函数不要自行改名、换参数或改变返回语义。确需修改时，先在团队群中说明：

- 修改原因；
- 原签名和新签名；
- 受影响的调用方；
- 是否改变引脚、Timer、UART、PWM 或中断资源。

得到协调后，在同一 PR 中同步修改头文件、实现、调用方、`docs/interfaces/` 和 `docs/hardware-resources.md`，并请受影响模块负责人复核。

### 不提交的内容

以下内容是生成物或个人配置，必须留在本机：

- `Objects/`、`Listings/`；
- `.uvgui.*`、`.uvguix.*`；
- `.vscode/`；
- `.obj`、`.lst`、`.m51`、`.hex`、`.lnp`、构建日志等 `.gitignore` 已列出的产物。

## 源码编码约定

- 中文 Markdown 文档使用 UTF-8。
- 新增或修改的 `.c/.h` 只写 ASCII 代码和英文注释，以兼容 C51 及既有 ANSI/GBK 文件。
- 不要对导入的 `.c/.h` 做批量编码转换、换行重写或“统一格式”；厂商来源文件的原字节编码和文件头必须保留。

## 八人分工

以下分工来自团队已确认的人员分工文档，不增加或替换人员：

| 方向 | 成员 | 主要职责 |
| --- | --- | --- |
| 硬件 / PCB | 程福鑫、胡智翔 | 阅读原项目原理图和 PCB；理清模块连接；负责 STC8H8K64U 最小系统、电源、舵机接口、下载/串口接口、外围模块、PCB 检查、打板、焊接和硬件排错 |
| 软件驱动层 / LIB 层 | 梅卫聪、张焰强、李兆鑫 | GPIO、Timer、PWM、UART、舵机、延时、传感器和其他外设驱动；向上层提供稳定接口 |
| 软件应用层 | 赵子博、徐军 | 前进、后退、转向、站立、趴下、动作组合、模式切换、状态控制及其他上层功能 |
| 项目整合 | 牛迈 | 工程结构、接口协调、资源分配、代码合并、模块联调、进度、最终测试和模块冲突处理 |

每位成员都必须自己记录真实调试证据，保留手写项目错题本/Bug 记录本。至少记录现象、最初判断、排查步骤、真实根因、解决办法，以及下次优先检查项；重点是“怎样定位”，不是只写最终答案。不得让 AI 生成或补写并未真实发生的个人 Bug 记录，本仓库也不伪造任何成员的错题本内容。

## 当前限制

- 第一版仅支持 `STOP`、`STAND`、`CROUCH` 三种占位姿态；前进、后退、左转、右转和打招呼返回 `APP_ERR_NOT_READY`。
- 三个姿态数组、舵机通道、方向、零位、机械极限和 1000–2000 us 脉宽都是初始占位值，连接实物前必须逐路校准。
- `STOP` 会发送 90° 中立占位姿态，不会断开舵机电源，也不会关闭 PCA9685 PWM。
- 蓝牙发送、超声波采样、电池采样、蜂鸣器发声和灯控制尚无确认的硬件分配，Driver 操作接口返回 `DRIVER_ERR_NOT_READY`；它们的 `Init` 暂时只是无动作占位。
- PCA9685 使用 P3.2/P3.3 软件 I2C，实物总线需要外部上拉；未接硬件时不要把初始化成功或失败误解为整机已验证。
- 电池阈值、超声波停止距离和安全传感器参数尚未实物标定。
