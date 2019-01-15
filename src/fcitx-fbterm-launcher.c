#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  uid_t uid = getuid();

  if (setuid(0) == -1) {
    perror("setuid");
    return EXIT_FAILURE;
  }

  const struct rlimit limit = {1 * 60, 1 * 60};
  int err = setrlimit(RLIMIT_CPU, &limit);
  if (err) {
    perror("setrlimit");
    return EXIT_FAILURE;
  }

  if (setuid(uid) == -1) {
    perror("setuid");
    return EXIT_FAILURE;
  }

  char* args[100] = {NULL};
  int args_len = argc;
  if (args_len >= sizeof(args) / sizeof(args[0])) {
    args_len = sizeof(args) / sizeof(args[0]) - 1;
  }
  memcpy(&args[0], &argv[0], args_len * sizeof(argv[0]));
  args[0] = "/usr/bin/fcitx-fbterm";
  execvp(args[0], args);

  return EXIT_FAILURE;
}
