# 9_hanwudadi 嵌入式小组

这里用于保存小组的嵌入式系统学习记录、实验代码和项目代码，不限定具体芯片或开发平台。

## 分支说明

- `main`：可以正常使用的稳定版本。
- `develop`：日常开发与功能整合。
- `feature/功能名`：开发新功能时，从 `develop` 创建的临时分支。

## 推荐协作流程

1. 开始工作前切换到 `develop` 并拉取最新代码。
2. 创建自己的功能分支，例如 `feature/led-driver`。
3. 完成修改后提交并推送到 GitHub。
4. 在 GitHub 创建 Pull Request，合并回 `develop`。
5. 项目测试稳定后，再把 `develop` 合并到 `main`。

## 提交信息示例

- `feat: 添加串口驱动`
- `fix: 修复按键消抖问题`
- `docs: 补充接线说明`
- `refactor: 整理定时器代码`

## 建议的项目目录

每个项目建立独立目录，并在目录内放置说明文档：

```text
projects/
  project-name/
    README.md
    src/
    include/
    docs/
```

请勿提交密码、Token、Wi-Fi 密码等敏感信息。
