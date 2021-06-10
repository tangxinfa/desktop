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
  status = ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                                 : WEXITSTATUS(status));
  return status;
}

const char *keymaps_start[] = {
    NULL,                    // Placeholder of the keymaps header
    "keycode 58 = Control",  // Caps-Lock as Control
    "keycode 125 = Alt",     // Left Meta(Win) as Alt
    "keycode 100 = Alt",     // Right Alt as Alt
    NULL                     // Terminate with NULL
};

const char *keymaps_stop[] = {
    NULL,                      // Placeholder of the keymaps header
    "keycode 58 = Caps_Lock",  // Caps-Lock
    "keycode 100 = AltGr\nalt	keycode 100 = Compose",  // Right Alt
    "keycode 125 = nul",                                 // Left Meta(Win)
    NULL                                                 // Terminate with NULL
};

const char *keymaps_header() {
  static char header[1024] = {'\0'};
  if (header[0] == '\0') {
    FILE *f = popen("dumpkeys | head -1", "r");
    if (f != NULL) {
      fgets(header, sizeof(header), f);
      pclose(f);
    }
    int length = strlen(header);
    if (length > 0 && header[length - 1] == '\n') {
      header[length - 1] = '\0';
    }
  }

  return header;
}

#define KEYMAPS_ENABLED_PATTERN " *keycode +58 *= *Control"

int main(int argc, char *argv[]) {
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

  keymaps_start[0] = keymaps_stop[0] = keymaps_header();

  if (strcmp(argv[1], "status") == 0) {
    int status = system("dumpkeys -k | grep -E '" KEYMAPS_ENABLED_PATTERN
                        "' >/dev/null 2>&1");
    status = ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                                   : WEXITSTATUS(status));
    printf("%s\n", (status == 0 ? "started" : "stoped"));
    return status;
  }

  const char **keymaps = keymaps_start;
  if (strcmp(argv[1], "stop") == 0) {
    keymaps = keymaps_stop;
  }

  return execute("loadkeys -", keymaps);
}
