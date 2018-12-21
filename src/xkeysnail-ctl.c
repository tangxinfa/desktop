#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  if (argc != 2 ||
      (strcmp(argv[1], "start") != 0 && strcmp(argv[1], "stop") != 0 &&
       strcmp(argv[1], "status") != 0)) {
    fprintf(stderr, "Usage: %s <start|stop|status>\n", argv[0]);
    return EXIT_FAILURE;
  }

  if (setuid(0) == -1) {
    perror("setuid");
    return EXIT_FAILURE;
  }

  int status = EXIT_FAILURE;
  const char* display = getenv("DISPLAY");

  if (strcmp(argv[1], "start") == 0) {
    if (display && display[0] != '\0') {
      status = system(
          "xhost +SI:localuser:root >/dev/null 2>&1; setcapslock off; setleds "
          "-caps; "
          "xkeysnail "
          "~/.config/xkeysnail/config.py --quiet --watch >/dev/null 2>&1 &");
      return ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                                   : WEXITSTATUS(status));
    }
    return EXIT_FAILURE;
  } else if (strcmp(argv[1], "stop") == 0) {
    // setcapslock will crash with core in Non-X environments.
    if (display && display[0] != '\0') {
      system("setcapslock off");
    }
    status = system("setleds -caps; killall xkeysnail");
    return ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                                  : WEXITSTATUS(status));
  } else if (strcmp(argv[1], "status") == 0) {
    status = system("pgrep -x xkeysnail >/dev/null 2>&1");
    status = ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                                    : WEXITSTATUS(status));
    printf("%s\n", (status == 0 ? "started" : "stoped"));
    return status;
  }

  return EXIT_FAILURE;
}
