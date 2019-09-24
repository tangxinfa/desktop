#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <regex.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <time.h>
#include <unistd.h>

const char* monitor_lock_file = "/tmp/bluetooth-mobile-monitor.lock";
const char* status_file = "/tmp/bluetooth-mobile-ctl.status";

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
    const char* path = "/tmp/bluetooth-mobile-monitor.log";
    file = fopen(path, "a");
    if (!file) {
      logging(stderr, "Open %s failed: %d", path, errno);
    } else {
      logging(stderr, "Open %s succeed", path);
    }
  }
  return file;
}

bool bluetooth_mobile_detected(const char* device) {
  char command[PATH_MAX] = {'\0'};
  snprintf(command, sizeof(command), "l2ping -c 1 '%s' >/dev/null 2>&1",
           device);
  int status = system(command);
  status = ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                                 : WEXITSTATUS(status));
  logging(stderr, "Bluetooth mobile device %s %s", device,
          (status == EXIT_SUCCESS ? "detected" : "not detected"));
  static int old_status = -1;
  if (old_status != status) {
    FILE* file = fopen(status_file, "w");
    if (!file) {
      logging(stderr, "Create status file %s failed: %d", status_file, errno);
    } else {
      fputs((status == EXIT_SUCCESS ? "on" : "off"), file);
      fclose(file);
    }
    old_status = status;
  }
  return (status == EXIT_SUCCESS);
}

bool bluetooth_mobile_wait(const char* device) {
  logging(stderr, "Wait until bluetooth mobile device %s appear ...", device);
  while (!bluetooth_mobile_detected(device)) {
    sleep(1);
  }

  logging(stderr, "Wait until bluetooth mobile device %s disappear ...",
          device);
  char command[PATH_MAX] = {'\0'};
  snprintf(command, sizeof(command), "l2ping -d 2 '%s' >/dev/null 2>&1",
           device);
  system(command);

  time_t begin = time(NULL);
  for (int i = 0; i < 3; ++i) {
    if (bluetooth_mobile_detected(device)) {
      return false;
    } else if (time(NULL) - begin > 7) {
      break;
    }
  }

  return true;
}

int bluetooth_mobile_monitor_lock() {
  const int lock_fd =
      open(monitor_lock_file, O_CREAT | O_RDWR | O_CLOEXEC, 0666);
  if (lock_fd < 0) {
    const int old_errno = errno;
    logging(stderr, "Create lock %s failed: %d", monitor_lock_file, old_errno);
    return old_errno;
  }
  if (flock(lock_fd, LOCK_EX | LOCK_NB) == -1) {
    const int old_errno = errno;
    if (old_errno != EWOULDBLOCK) {
      logging(stderr, "Acquire lock %s failed: %d", monitor_lock_file,
              old_errno);
    }
    return old_errno;
  }

  return 0;
}

int bluetooth_mobile_monitor(const char* device) {
  const int error = bluetooth_mobile_monitor_lock();
  if (error != 0) {
    return error;
  }
  do {
    if (bluetooth_mobile_wait(device)) {
      logging(stderr, "Mobile far way, lock the desktop");
      system("~/bin/desktop-lock mobile-far-away");
    } else {
      logging(stderr, "Wait bluetooth mobile device %s failed", device);
    }
  } while (true);

  return 0;
}

int main(int argc, char* argv[]) {
  if (argc != 2 ||
      (strcmp(argv[1], "status") != 0 && strcmp(argv[1], "monitor") != 0)) {
    fprintf(stderr, "Usage: %s <status|monitor>\n", argv[0]);
    return EXIT_FAILURE;
  }

  const char* device = getenv("BLUETOOTH_MOBILE_DEVICE");
  if (!device) {
    logging(stderr,
            "error: BLUETOOTH_HEADSET_NAME environment variable not set");
    return EXIT_FAILURE;
  }

  if (setuid(0) == -1) {
    perror("setuid");
    exit(EXIT_FAILURE);
  }
  if (setgid(0) == -1) {
    perror("setgid");
    exit(EXIT_FAILURE);
  }

  if (strcmp(argv[1], "status") == 0) {
    char status[8] = {'\0'};
    FILE* file = fopen(status_file, "r");
    if (!file) {
      logging(stderr, "Open status file %s failed: %d", status_file, errno);
    } else {
      fgets(status, sizeof(status), file);
      fclose(file);
    }
    const bool on = strcmp(status, "on") == 0 &&
                    bluetooth_mobile_monitor_lock() == EWOULDBLOCK;
    printf("%s\n", (on ? "on" : "off"));
    return EXIT_SUCCESS;
  }

  return bluetooth_mobile_monitor(device);
}
