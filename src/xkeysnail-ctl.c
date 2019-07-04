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
  if (setgid(0) == -1) {
    perror("setgid");
    return EXIT_FAILURE;
  }

  int status = EXIT_FAILURE;
  const char* display = getenv("DISPLAY");

  if (strcmp(argv[1], "start") == 0) {
    if (display == NULL || display[0] == '\0') {
      setenv("DISPLAY", ":0", 1);
    }
    status = system(
        "xhost +SI:localuser:root >/dev/null 2>>/tmp/xkeysnail.log; "
        "xkeysnail ~/.config/xkeysnail/config.py --quiet --watch "
        ">/dev/null 2>>/tmp/xkeysnail.log &");
    status = ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                                   : WEXITSTATUS(status));
    // Wait until xkeysnail startup.
    if (status == EXIT_SUCCESS) {
      for (int i = 0; i < 3; ++i) {
        status = system("pgrep -x xkeysnail >/dev/null 2>&1");
        status = ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                                       : WEXITSTATUS(status));
        if (status == EXIT_SUCCESS) {
          break;
        }
        sleep(1);
      }
    }
    return status;
  } else if (strcmp(argv[1], "stop") == 0) {
    status = system("killall xkeysnail");
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
