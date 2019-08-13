#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <regex.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <time.h>
#include <unistd.h>

void logging(FILE* file, const char* fmt, ...) {
  time_t t;
  struct tm tm;
  time(&t);
  localtime_r(&t, &tm);
  char timestamp[26] = {'\0'};
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm);
  fprintf(file, "%s\t", timestamp);
  va_list args;
  va_start(args, fmt);
  int rc = vfprintf(file, fmt, args);
  va_end(args);
  fprintf(file, "\n");
  fflush(file);
  fsync(fileno(file));
}

FILE* logger() {
  static FILE* file = NULL;
  if (!file) {
    char path[PATH_MAX] = {'\0'};
    snprintf(path, sizeof(path), "%s/lid-monitor.log", getenv("HOME"));
    file = fopen(path, "a");
    if (!file) {
      logging(stderr, "Open %s failed: %d", path, errno);
    } else {
      logging(stderr, "Open %s succeed", path);
    }
  }
  return file;
}

// When LID closed the system should suspend, if not it means systemctl hang,
// force power off as a rescue to avoid computer damage by high temperature.
void poweroff() {
  const char* path = "/proc/sys/kernel/sysrq";
  FILE* file = fopen(path, "w");
  if (!file) {
    logging(stderr, "Open %s failed: %d", path, errno);
    return;
  }
  fputc('1', file);
  fclose(file);

  path = "/proc/sysrq-trigger";
  file = fopen(path, "w");
  if (!file) {
    logging(stderr, "Open %s failed: %d", path, errno);
    return;
  }
  fputc('o', file);
  fclose(file);
}

bool lid_opened() {
  const char* path = "/proc/acpi/button/lid/LID/state";
  FILE* file = fopen(path, "r");
  if (!file) {
    logging(stderr, "Open %s failed: %d, treat as lid closed", path, errno);
    return false;
  }
  char line[128] = {'\0'};
  fgets(line, sizeof(line), file);
  fclose(file);
  regex_t regex;
  int error = regcomp(&regex, "state: *open", 0);
  assert(!error);
  error = regexec(&regex, line, 0, NULL, 0);
  regfree(&regex);
  return error == 0;
}

bool lid_problem() {
  for (int i = 0; i < 30; ++i) {
    if (lid_opened()) {
      return false;
    }
    if (i > 10) {
      logging(logger(), "Laptop LID closed, why not suspend?");
    }
    sleep(1);
  }
  return true;
}

int lid_monitor() {
  const char* lock_file = "/tmp/lid-monitor.lock";
  const int lock_fd = open(lock_file, O_CREAT | O_RDWR | O_CLOEXEC, 0666);
  if (lock_fd < 0) {
    logging(stderr, "Create lock %s failed: %d", lock_file, errno);
    return errno;
  }
  if (flock(lock_fd, LOCK_EX | LOCK_NB) == -1) {
    if (errno != EWOULDBLOCK) {
      logging(stderr, "Acquire lock %s failed: %d", lock_file, errno);
    }
    return errno;
  }

  do {
    if (lid_problem()) {
      logging(logger(),
              "Laptop LID closed but system still running, rescue with force "
              "power off");
      poweroff();
    }

    sleep(10);
  } while (true);

  return 0;
}

int main(int argc, char* argv[]) {
  if (setuid(0) == -1) {
    perror("setuid");
    return EXIT_FAILURE;
  }
  if (setgid(0) == -1) {
    perror("setgid");
    return EXIT_FAILURE;
  }

  return lid_monitor();
}
