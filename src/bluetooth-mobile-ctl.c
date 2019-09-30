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

/**
 * Check if mobile device is reachable.
 *
 * @param device Device to check.
 *
 * @return true if reachable, false otherwise.
 */
bool bluetooth_mobile_reachable(const char* device) {
  char command[PATH_MAX] = {'\0'};
  snprintf(command, sizeof(command),
           "timeout 2 l2ping -r -t 1 -c 1 '%s' >/dev/null 2>&1", device);
  int status = system(command);
  status = ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                                 : WEXITSTATUS(status));
  logging(stderr, "Bluetooth mobile device %s %s", device,
          (status == EXIT_SUCCESS ? "reachable" : "unreachable"));
  return (status == EXIT_SUCCESS);
}

/**
 * Keepalive with mobile device.
 *
 * @param device Device to keepalive.
 */
void bluetooth_mobile_keepalive(const char* device) {
  logging(stderr, "Bluetooth mobile device %s keepalive ...", device);
  char command[PATH_MAX] = {'\0'};
  snprintf(command, sizeof(command), "l2ping -r -t 1 -d 2 '%s' >/dev/null 2>&1",
           device);
  system(command);
  logging(stderr, "Bluetooth mobile device %s keepalive interrupted", device);
}

/**
 * Check if mobile device is really faraway.
 *
 * @param device Device to check.
 *
 * @return true if really faraway, false otherwise.
 */
bool bluetooth_mobile_faraway(const char* device) {
  const int64_t interval_ms = 1 * 1000;
  const int64_t total_ms = 21 * 1000;

  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  const int64_t check_begin_ms = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

  while (true) {
    clock_gettime(CLOCK_MONOTONIC, &ts);
    const int64_t round_begin_ms = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

    if (bluetooth_mobile_reachable(device)) {
      return false;
    }

    clock_gettime(CLOCK_MONOTONIC, &ts);
    const int64_t round_end_ms = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

    if (round_end_ms - check_begin_ms > total_ms) {
      break;
    }

    const int64_t delay_ms = interval_ms - (round_end_ms - round_begin_ms);

    if (delay_ms > 0) {
      usleep(delay_ms * 1000);
    }
  }

  return true;
}

bool bluetooth_mobile_activate_desktop() {
  static char old_desktop_lock_id[64] = {'\0'};

  char new_desktop_lock_id[sizeof(old_desktop_lock_id)] = {'\0'};
  FILE* fp = popen("pidof vlock i3lock", "r");
  if (NULL == fp) {
    perror("popen");
    return false;
  }
  fgets(new_desktop_lock_id, sizeof(new_desktop_lock_id), fp);
  pclose(fp);

  // Activate the desktop if it running a new lock.
  const bool new_lock = strcmp(new_desktop_lock_id, old_desktop_lock_id) != 0;
  if (new_lock) {
    system("~/bin/desktop-activate");
  }

  strcpy(old_desktop_lock_id, new_desktop_lock_id);

  return new_lock;
}

bool bluetooth_mobile_status_file_write(bool faraway) {
  static int old_value = -1;
  if (old_value != faraway) {
    FILE* file = fopen(status_file, "w");
    if (!file) {
      logging(stderr, "Create status file %s failed: %d", status_file, errno);
    } else {
      fputs(faraway ? "off" : "on", file);
      fclose(file);
    }
    if (!faraway) {
      bluetooth_mobile_activate_desktop();
    }
    old_value = faraway;
    return true;
  }

  return false;
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

  bool enable_lock_desktop = false;

  do {
    const bool faraway = bluetooth_mobile_faraway(device);
    const bool status_changed = bluetooth_mobile_status_file_write(faraway);
    if (faraway) {
      if (status_changed && enable_lock_desktop) {
        logging(stderr, "Bluetooth mobile device %s faraway, lock the desktop",
                device);
        system("~/bin/desktop-lock mobile-faraway");
      }
      continue;
    }
    enable_lock_desktop = true;
    bluetooth_mobile_keepalive(device);
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
            "error: BLUETOOTH_MOBILE_DEVICE environment variable not set");
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
