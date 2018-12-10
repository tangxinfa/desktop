#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void daemonize() {
  int i;
  pid_t pid;

  if (pid = fork()) {
    int status = EXIT_FAILURE;
    waitpid(pid, &status, 0);
    status = WEXITSTATUS(status);
    exit(status);
  }

  setsid();
  signal(SIGHUP, SIG_IGN);

  if (pid = fork()) {
    int status = EXIT_FAILURE;
    waitpid(pid, &status, 0);
    status = WEXITSTATUS(status);
    exit(status);
  }

  umask(0);
}

int execute(const char *command, const char *input) {
  FILE *fp = popen(command, (input ? "w" : "r"));
  if (NULL == fp) {
    perror("popen");
    return EXIT_FAILURE;
  }
  if (input) {
    fputs(input, fp);
  } else {
    char line[512] = {'\0'};
    while (fgets(line, sizeof(line), fp)) {
      fputs(line, stdout);
    }
  }
  int status = pclose(fp);
  if (-1 == status) {
    perror("pclose");
    return EXIT_FAILURE;
  }
  return WEXITSTATUS(status);
}

const char *keymaps =
    "keymaps 0-2,4-6,8-9,12 # By run `sudo dumpkeys | head -1`\n"
    "keycode 58 = Control   # Caps-Lock as Control\n"
    "keycode 125 = Alt      # Win as Alt\n";

int main(int argc, char *argv[]) {
  if (setuid(0) == -1) {
    perror("setuid");
    fprintf(stderr, "Permission error: %s\nPlease execute: sudo %s setup\n",
            argv[0], argv[0]);
    return EXIT_FAILURE;
  }
  if (argc != 2 ||
      (strcmp(argv[1], "on") != 0 && strcmp(argv[1], "off") != 0 &&
       strcmp(argv[1], "dump") != 0 && strcmp(argv[1], "setup") != 0)) {
    fprintf(stderr, "Usage: %s <on|off|dump|setup>\n", argv[0]);
    return EXIT_FAILURE;
  }

  if (strcmp(argv[1], "setup") == 0) {
    char command[512] = {'\0'};
    snprintf(command, sizeof(command),
             "chown root:root '%s' && chmod 4755 '%s'", argv[0], argv[0]);
    return execute(command, NULL);
  }

  daemonize();

  if (strcmp(argv[1], "on") == 0) {
    return execute("loadkeys -", keymaps);
  } else if (strcmp(argv[1], "off") == 0) {
    return execute("loadkeys -d", NULL);
  }
  return execute("dumpkeys", NULL);
}
