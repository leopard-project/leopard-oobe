#include "xtuiapi.h"   /* xapi_* / WSTR；libsys.h：enter_syscall / XAPI_INPUT_NO_ECHO */

#define USER_CAP 64

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

    xapi_Printf("LeopardOobe: first boot setup\n");

    char username[USER_CAP];
    char password[USER_CAP];

    xapi_Printf("Username: ");
    read_line(username, sizeof(username), 0);

    xapi_Printf("Password: ");
    read_line(password, sizeof(password), XAPI_INPUT_NO_ECHO);

    uint64_t result = xapi_UserCreateFirst((WSTR)username, (WSTR)password);
    wipe(password, sizeof(password));

    if (result != 0) {
        xapi_Printf("LeopardOobe: account setup failed\n");
        return 1;   /* 退出 → respawn getty → 仍需要 OOBE → 重进 */
    }

    xapi_Printf("LeopardOobe: account created\n");
    return 0;       /* 退出 → respawn getty → 已完成 OOBE → 进登录 */
}

