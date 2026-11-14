# PROJECT KNOWLEDGE BASE — leopard-oobe

首次启动建号（out-of-box experience）。建号后初始化 `/users/<user>/`，并写
`/users/<user>/config`（`shell=` / `path=`）。主目录约定是 `/users`，不建 `/home`。

## 构建
    python3 gen_ninja.py && ninja build   # → out/leopard-oobe.elf
    ninja pack

- 静态 ELF，依赖 `../XACT-CRT`（`include/` + `lib/x86_64/`）。
