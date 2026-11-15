/*
 * leopard-oobe — first-boot account setup on the PTY.
 *
 * Runs as a child of leopard-getty on fd 0/1/2.  Prompts use the PTY line
 * discipline; the password field disables ECHO via termios.  No XAPI_INPUT.
 */
#include "xtuiapi.h"
#include "libsys.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define USER_CAP      64
#define CONFIG_CAP    512
#define PATH_CAP      256
#define DEFAULT_SHELL "/apps/system/lpsh.elf"
#define DEFAULT_PATH  "/apps/system:/apps/leopard"

#ifndef TCGETS
#define TCGETS 0x5401
#endif
#ifndef TCSETS
#define TCSETS 0x5402
#endif
#ifndef ECHO
#define ECHO 0000010
#endif

struct oobe_termios {
        uint32_t c_iflag;
        uint32_t c_oflag;
        uint32_t c_cflag;
        uint32_t c_lflag;
        uint8_t  c_line;
        uint8_t  c_cc[20];
};

static void out(const char *text)
{
        if (text != NULL && text[0] != '\0')
                (void)write(1, text, strlen(text));
}

static int read_line(char *buffer, size_t capacity, int no_echo)
{
        struct oobe_termios saved;
        struct oobe_termios term;
        int    restore = 0;
        size_t length = 0;

        if (buffer == NULL || capacity == 0) return -1;
        buffer[0] = '\0';

        if (no_echo && ioctl(0, TCGETS, &term) == 0) {
                saved = term;
                term.c_lflag &= ~((uint32_t)ECHO);
                if (ioctl(0, TCSETS, &term) == 0) restore = 1;
        }

        for (;;) {
                char    ch;
                ssize_t got = read(0, &ch, 1);
                if (got < 0) {
                        if (errno == EINTR) continue;
                        break;
                }
                if (got == 0) break;
                if (ch == '\n' || ch == '\r') break;
                if (ch == '\b' || ch == 0x7f) {
                        if (length > 0) length--;
                        continue;
                }
                if (length + 1 < capacity) buffer[length++] = ch;
        }
        buffer[length] = '\0';

        if (restore) (void)ioctl(0, TCSETS, &saved);
        return (int)length;
}

static void wipe(char *buffer, size_t length)
{
        while (length-- > 0) buffer[length] = '\0';
}

int main(int argc, char *argv[], char *envp[])
{
        (void)argc;
        (void)argv;

        out("Leopard OOBE: first boot setup\n");

        char username[USER_CAP];
        char password[USER_CAP];

        out("Username: ");
        read_line(username, sizeof(username), 0);

        out("Password: ");
        read_line(password, sizeof(password), 1);
        out("\n");

        uint64_t result = enter_syscall(XAPI_USER_CREATE_FIRST, (uint64_t)username,
                                        (uint64_t)password, 0, 0, 0, 0);
        wipe(password, sizeof(password));

        if (result != 0) {
                out("Leopard OOBE: account setup failed\n");
                return 1;
        }

        char config_path[PATH_CAP];
        snprintf(config_path, sizeof(config_path), "/users/%s/config", username);

        char config[CONFIG_CAP];
        snprintf(config, sizeof(config),
                 "shell=%s\n"
                 "path=%s\n",
                 DEFAULT_SHELL, DEFAULT_PATH);

        int64_t wrote = (int64_t)xapi_WriteFile((WSTR)config_path, config, strlen(config), 0);
        if (wrote <= 0) {
                out("Leopard OOBE: failed to write user config\n");
                return 1;
        }

        out("Leopard OOBE: account created\n");
        return 0;
}
