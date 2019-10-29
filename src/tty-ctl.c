#include <errno.h>
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

int get_graphic_display(char* display, size_t size) {
  FILE* fp = popen(
      "cat /proc/`pidof Xorg`/cmdline | tr \"\\0\" \"\\n\" | grep : | head "
      "-1",
      "r");
  if (NULL == fp) {
    perror("popen");
    return errno;
  }
  memset(display, '\0', size);
  fgets(display, size, fp);
  int status = pclose(fp);
  status = ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                                 : WEXITSTATUS(status));
  char* eol = strchr(display, '\n');
  if (eol) {
    *eol = '\0';
  }
  if (status == 0 && display[0] == '\0') {
    status = EXIT_FAILURE;
  }
  return status;
}

int main(int argc, char* argv[]) {
  int to_tty = 0;
  if (argc != 2 ||
      (strcmp(argv[1], "active") != 0 && strcmp(argv[1], "graphic") != 0 &&
       strcmp(argv[1], "other") != 0 && strcmp(argv[1], "activate") != 0 &&
       strcmp(argv[1], "lock") != 0 && strcmp(argv[1], "capslockoff") != 0 &&
       strcmp(argv[1], "graphic-display") != 0 &&
       strcmp(argv[1], "dmesg-off") != 0)) {
    to_tty = atoi(argc == 2 ? argv[1] : "0");
    if (to_tty == 0) {
      fprintf(stderr,
              "Usage: %s "
              "<active|graphic|other|activate|lock|capslockoff|graphic-display|"
              "N|>\n"
              "\tactive          Print active tty.\n"
              "\tgraphic         Print graphic tty.\n"
              "\tother           Swith to other tty.\n"
              "\t                Create tty if $TTY_CTL_OTHER_CREATE is set.\n"
              "\tactivate        Active caller's tty.\n"
              "\tlock            Lock all tty.\n"
              "\tcapslockoff     Set CapsLock off.\n"
              "\tgraphic-display Print graphic display.\n"
              "\tdmesg-off       Disable printing messages to console.\n"
              "\tN               Switch to tty N.\n",
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
  if (strcmp(argv[1], "capslockoff") == 0) {
    // Non-X environments.
    system(
        "setleds -caps < /dev/tty`cat /sys/class/tty/tty0/active | awk -Ftty "
        "'{print $2}'`");
    char display[32] = {'\0'};
    if (0 == get_graphic_display(display, sizeof(display))) {
      // X environments.
      // setcapslock will crash with core in Non-X environments, setcapslock
      // sometimes not work, must use xdotool to let Caps_Lock key up first.
      snprintf(command, sizeof(command),
               "xdpyinfo -display '%s'>/dev/null 2>&1", display);
      status = system(command);
      status = ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                                     : WEXITSTATUS(status));
      if (0 == status) {
        snprintf(command, sizeof(command),
                 "export DISPLAY=%s; xdotool key Caps_Lock; "
                 "setcapslock off",
                 display);
        system(command);
      }
    }

    return EXIT_SUCCESS;
  }

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
        "(readlink /proc/`pidof Xorg Xwayland -s`/fd/0 | awk "
        "-F/dev/tty '{print $2}') 2>/dev/null");
    status = ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                                   : WEXITSTATUS(status));
    return status;
  }

  if (strcmp(argv[1], "graphic-display") == 0) {
    char display[32] = {'\0'};
    status = get_graphic_display(display, sizeof(display));
    if (status == 0) {
      printf("%s\n", display);
    }
    return status;
  }

  if (strcmp(argv[1], "dmesg-off") == 0) {
    system("dmesg --console-off");
    return EXIT_SUCCESS;
  }

  const char* my_tty = getenv("XDG_VTNR");
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
