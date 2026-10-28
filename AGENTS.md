# PROJECT KNOWLEDGE BASE — leopard-oobe

首次启动建号（out-of-box experience）。建号后初始化 `/home/<user>/.config/` 并写默认 `shell.conf`。

## 构建
    python3 gen_ninja.py && ninja build   # → out/leopard-oobe.elf
    ninja pack

- 静态 ELF，依赖 `../leopard-sdk`。
