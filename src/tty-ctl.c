#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int tty_ready(int tty) {
  char command[255] = {'\0'};
  snprintf(
      command, sizeof(command),
      "ps aux | grep tty%d | grep -v '\\(agetty\\|grep\\)' >/dev/null 2>&1",
      tty);
  int status = system(command);
  status = ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                                 : WEXITSTATUS(status));
  return status;
}

int tty_wait(int tty) {
  char command[255] = {'\0'};
  snprintf(command, sizeof(command),
           "cat /sys/class/tty/tty0/active | grep -E 'tty%d$' >/dev/null 2>&1",
           tty);
  int status = EXIT_FAILURE;
  for (int i = 0; i < 10; ++i) {
    status = system(command);
    status = ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                                   : WEXITSTATUS(status));
    if (status == 0) {
      break;
    }
    sleep(1);
  }
  return status;
}

int main(int argc, char *argv[]) {
  int to_tty = 0;
  if (argc != 2 ||
      (strcmp(argv[1], "active") != 0 && strcmp(argv[1], "graphic") != 0 &&
       strcmp(argv[1], "other") != 0 && strcmp(argv[1], "activate") != 0 &&
       strcmp(argv[1], "lock") != 0)) {
    to_tty = atoi(argc == 2 ? argv[1] : "0");
    if (to_tty == 0) {
      fprintf(stderr,
              "Usage: %s "
              "<active|graphic|other|activate|lock|"
              "N>\n"
              "\tactive   Print active tty.\n"
              "\tgraphic  Print graphic tty.\n"
              "\tother    Swith to other tty.\n"
              "\t         Create tty if $TTY_CTL_OTHER_CREATE set.\n"
              "\tactivate Active caller's tty.\n"
              "\tlock     Lock all tty.\n"
              "\tN        Switch to tty N.\n",
              argv[0]);
      return EXIT_FAILURE;
    }
  }

  if (setuid(0) == -1) {
    perror("setuid");
    return EXIT_FAILURE;
  }
  if (setgid(0) == -1) {
    perror("setgid");
    return EXIT_FAILURE;
  }

  if (strcmp(argv[1], "lock") == 0) {
    setenv("TERM", "linux", 1);
    system(
        "openvt -s -w -- bash -c 'if ! pidof vlock >/dev/null 2>&1; then "
        "setterm --blank 1; vlock -a; setterm --blank 15; fi'");
    return EXIT_SUCCESS;
  }

  int status = EXIT_FAILURE;
  char command[255] = {'\0'};
  if (to_tty != 0) {
    snprintf(command, sizeof(command), "chvt %d", to_tty);
    status = system(command);
    status = ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                                   : WEXITSTATUS(status));
    if (0 == status) {
      tty_wait(to_tty);
    }
    return status;
  }

  if (strcmp(argv[1], "active") == 0) {
    system("cat /sys/class/tty/tty0/active | awk -Ftty '{print $2}'");
    return EXIT_SUCCESS;
  }

  if (strcmp(argv[1], "graphic") == 0) {
    status = system(
        "(cat /proc/`pgrep -x 'Xorg|Xwayland' | head -1`/environ | strings | "
        "grep XDG_VTNR | head -1 | awk -F= '{print $2}') 2>/dev/null");
    status = ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                                   : WEXITSTATUS(status));
    return status;
  }

  const char *my_tty = getenv("XDG_VTNR");
  if (my_tty == NULL) {
    fputs("Warning: $XDG_VTNR not exists, default is \"1\"\n", stderr);
    my_tty = "1";
  }

  if (strcmp(argv[1], "activate") == 0) {
    snprintf(command, sizeof(command), "chvt %s", my_tty);
    status = system(command);
    status = ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                                   : WEXITSTATUS(status));
    return 0 == status ? tty_wait(atoi(my_tty)) : status;
  }

  const int other_tty = (strcmp(my_tty, "1") == 0 ? 2 : 1);
  if (getenv("TTY_CTL_OTHER_CREATE") || tty_ready(other_tty) == 0) {
    snprintf(command, sizeof(command), "chvt %d", other_tty);
    status = system(command);
    status = ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                                   : WEXITSTATUS(status));
    return 0 == status ? tty_wait(other_tty) : status;
  }

  return EXIT_FAILURE;
}
