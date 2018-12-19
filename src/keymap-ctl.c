#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int execute(const char *command, const char *inputs[]) {
  FILE *fp = popen(command, (inputs ? "w" : "r"));
  if (NULL == fp) {
    perror("popen");
    return EXIT_FAILURE;
  }
  if (inputs) {
    for (int i = 0; inputs[i]; ++i) {
      fputs(inputs[i], fp);
      fputs("\n", fp);
    }
  }
  int status = pclose(fp);
  if (-1 == status) {
    perror("pclose");
    return EXIT_FAILURE;
  }
  return WEXITSTATUS(status);
}

const char *keymaps[] = {
    "keymaps 0-2,4-6,8-9,12",  // By run: sudo dumpkeys | head -1
    "keycode 58 = Control",    // Caps-Lock as Control
    "keycode 125 = Alt",       // Win as Alt
    NULL                       // Terminate with NULL
};

#define ENABLED_PATTERN " *keycode +58 *= *Control"

int main(int argc, char *argv[]) {
  if (argc != 2 ||
      (strcmp(argv[1], "start") != 0 && strcmp(argv[1], "stop") != 0 && strcmp(argv[1], "status") != 0)) {
    fprintf(stderr, "Usage: %s <start|stop|status>\n", argv[0]);
    return EXIT_FAILURE;
  }

  if (setuid(0) == -1) {
    perror("setuid");
    return EXIT_FAILURE;
  }

  if (strcmp(argv[1], "start") == 0) {
    return execute("loadkeys -", keymaps);
  } else if (strcmp(argv[1], "stop") == 0) {
    return execute("loadkeys -d", NULL);
  }

  const int status =
      execute("dumpkeys -k | grep -E '" ENABLED_PATTERN "'", NULL);
  printf("%s\n", (status == 0 ? "started" : "stoped"));
  return status;
}
