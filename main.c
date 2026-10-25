#include "xtuiapi.h"   /* xapi_* / WSTR；libsys.h：enter_syscall / XAPI_INPUT_NO_ECHO */

#define USER_CAP      64
#define CONFIG_CAP    512
#define PATH_CAP      256
#define DEFAULT_SHELL "/apps/system/lpsh.elf"
#define DEFAULT_PATH  "/apps/system:/apps/leopard"

static inline void read_line(char *buf, size_t cap, uint64_t flags)
{
    if (buf == NULL || cap == 0)
        return;
    buf[0] = '\0';
    enter_syscall(XAPI_INPUT, (uint64_t)buf, (uint64_t)cap, flags, 0, 0, 0);
}

static inline void wipe(char *buf, size_t n)
{
    while (n-- > 0)
        buf[n] = '\0';
}

int main(int argc, char *argv[], char *envp[])
{
    (void)argc;
    (void)argv;
    (void)envp;

    xapi_Printf("Leopard OOBE: first boot setup\n");

    char username[USER_CAP];
    char password[USER_CAP];

    xapi_Printf("Username: ");
    read_line(username, sizeof(username), 0);

    xapi_Printf("Password: ");
    read_line(password, sizeof(password), XAPI_INPUT_NO_ECHO);

    uint64_t result = xapi_UserCreateFirst((WSTR)username, (WSTR)password);
    wipe(password, sizeof(password));

    if (result != 0) {
        xapi_Printf("Leopard OOBE: account setup failed\n");
        return 1;   /* 退出 → respawn getty → 仍需要 OOBE → 重进 */
    }

    /* 建号成功后，把 shell / path 两个环境变量写进 /users/<用户名>/config */
    char config_path[PATH_CAP];
    snprintf(config_path, sizeof(config_path), "/users/%s/config", username);

    char config[CONFIG_CAP];
    snprintf(config, sizeof(config),
             "shell=%s\n"
             "path=%s\n",
             DEFAULT_SHELL, DEFAULT_PATH);

    int64_t wrote = (int64_t)xapi_WriteFile((WSTR)config_path, config, strlen(config), 0);
    if (wrote <= 0) {
        xapi_Printf("Leopard OOBE: failed to write user config\n");
        return 1;
    }

    xapi_Printf("Leopard OOBE: account created\n");
    return 0;       /* 退出 → respawn getty → 已完成 OOBE → 进登录 */
}
