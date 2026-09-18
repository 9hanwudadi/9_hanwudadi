# 第三方资料说明

`docs/hardware/` 中的商品资料截图、数据手册、厂商指南、示例代码压缩包和 LU-ASR 工程文件由项目成员提供，用于本学生项目的硬件选型、驱动开发和联调参考。

仓库负责人于 2026-09-18 提供这些文件并要求在本项目公开仓库中集中保存。该项目内授权记录不替代原权利人的许可；没有上游许可记录的文件均明确标为“未确认”。

- 相关商标、产品图片、文档和示例代码的权利归各自权利人所有。
- 收录文件不表示本项目取得了统一的开源许可，也不表示项目团队为资料内容、参数或兼容性提供保证。
- 商品页截图可能混合多个型号或随销售页面更新；必须以实际购买模块的丝印、版本和厂家资料为准。
- 仓库负责人负责确认其所提供文件适合公开；若后续无法确认公开依据，或收到权利人要求，应删除对应副本并改为指向官方来源的链接。
- 本项目自行生成的 A4 分页 PDF 只是对用户提供长截图的排版转换，不改变原始资料的权利归属。

## 逐文件来源与许可状态

| 仓库文件 | 来源记录 | 权利或许可状态 | 本仓库处理 |
| --- | --- | --- | --- |
| `module-list.png` | 项目成员提供的模块清单截图 | 项目内使用由仓库负责人确认 | 原样保存 |
| `power-lm2596/LM2596-product-screenshot.jpg` | 项目成员提供的商品页长截图；卖家和页面 URL 未记录 | 上游再分发许可未确认 | 原样保存，并生成 A4 分页版本 |
| `power-lm2596/LM2596-product-guide.pdf` | 由上一行截图生成 | 原截图权利状态不变 | 仅分页；PDF 元数据把项目标为转换者，不标为原作者 |
| `bluetooth-jdy-series/JDY-series-mixed-product-screenshot.jpg` | 项目成员提供的商品页长截图；混有 JDY-31、JDY-33、JDY-23 | 上游再分发许可未确认 | 原样保存，并生成带混合型号标题的 A4 版本 |
| `bluetooth-jdy-series/JDY-series-mixed-product-guide.pdf` | 由上一行截图生成 | 原截图权利状态不变 | 仅分页；PDF 元数据把项目标为转换者，不标为原作者 |
| `pca9685/Adafruit-16-channel-PWM-servo-shield-guide.pdf` | Adafruit Learning System，lady ada；[官方在线指南](https://learn.adafruit.com/adafruit-16-channel-pwm-slash-servo-shield) | 所提供 PDF 未附独立许可记录 | 原样保存；优先查阅官方在线版本 |
| `pca9685/PCA9685-sample-code.zip` | 外层资料包中的 `AdafruitPWMServoDriverLibrary.zip` | 内层 `license.txt`：BSD，Copyright 2012 Adafruit Industries；许可文本保留在压缩包中 | 原样保存 |
| `pca9685/PCA9685-reference-20141121.pdf` | 项目成员提供的 PCA9685 原理图/参考图；原始站点未记录 | 作者与再分发许可未确认 | 原样保存；芯片参数另查 [NXP 官方 PCA9685 数据手册](https://www.nxp.com/docs/en/data-sheet/PCA9685.pdf) |
| `pca9685/HW-170-PCA9685-product-manual.jpg` | 项目成员提供的 HW-170 产品说明长图；厂家和页面 URL 未记录 | 上游再分发许可未确认 | 原样保存 |
| `voice-lu-asr01/LU-ASR01-quick-start-guide.pdf` | 项目成员提供的 LU-ASR01 快速使用说明；原始站点未记录 | 作者与再分发许可未确认 | 原样保存 |
| `voice-lu-asr01/LU-ASR01-multi-command-IO-example.hd` | 项目成员提供的 LU-ASR01 多控命令与 IO 初始化工程 | 作者与再分发许可未确认 | 原字节保存 |
