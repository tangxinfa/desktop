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
  status = (status == -1 ? EXIT_FAILURE : WEXITSTATUS(status));
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
    status = (status == -1 ? EXIT_FAILURE : WEXITSTATUS(status));
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
      (strcmp(argv[1], "get") != 0 && strcmp(argv[1], "get-active") != 0 &&
       strcmp(argv[1], "next") != 0 && strcmp(argv[1], "next-create") != 0 &&
       strcmp(argv[1], "activate") != 0 && strcmp(argv[1], "lock") != 0)) {
    to_tty = atoi(argv[1]);
    if (to_tty == 0) {
      fprintf(stderr,
              "Usage: %s <get|get-active|next|next-create|activate|lock|N>\n\tN\tTTY number "
              "to switch\n",
              argv[0]);
      return EXIT_FAILURE;
    }
  }

  if (setuid(0) == -1) {
    perror("setuid");
    return EXIT_FAILURE;
  }

  if (strcmp(argv[1], "lock") == 0) {
    system(
        "openvt -s -w -- bash --noediting --noprofile --norc -c 'setterm "
        "--blank 1; vlock -a; setterm --blank 15'");
    return EXIT_SUCCESS;
  }

  int status = EXIT_FAILURE;
  char command[255] = {'\0'};
  if (to_tty != 0) {
    snprintf(command, sizeof(command), "chvt %d", to_tty);
    status = system(command);
    status = (status == -1 ? EXIT_FAILURE : WEXITSTATUS(status));
    if (0 == status) {
      tty_wait(to_tty);
    }
    return status;
  }

  if (strcmp(argv[1], "get-active") == 0) {
    system("cat /sys/class/tty/tty0/active | awk -Ftty '{print $2}'");
    return EXIT_SUCCESS;
  }

  const char *my_tty = getenv("XDG_VTNR");
  if (my_tty == NULL) {
    fputs("Warning: $XDG_VTNR not exists, default is \"1\"\n", stderr);
    my_tty = "1";
  }

  if (strcmp(argv[1], "get") == 0) {
    printf("%s\n", my_tty);
    return EXIT_SUCCESS;
  }

  if (strcmp(argv[1], "activate") == 0) {
    snprintf(command, sizeof(command), "chvt %s", my_tty);
    status = system(command);
    status = (status == -1 ? EXIT_FAILURE : WEXITSTATUS(status));
    return 0 == status ? tty_wait(atoi(my_tty)) : status;
  }


  const int next_tty = (strcmp(my_tty, "1") == 0 ? 2 : 1);

  if (strcmp(argv[1], "next-create") == 0 || tty_ready(next_tty) == 0) {
    snprintf(command, sizeof(command), "chvt %d", next_tty);
    status = system(command);
    status = (status == -1 ? EXIT_FAILURE : WEXITSTATUS(status));
    return 0 == status ? tty_wait(next_tty): status;
  }
  return EXIT_FAILURE;
}
