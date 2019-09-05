#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int xkeysnail_start() {
  const char* display = getenv("DISPLAY");
  if (display == NULL || display[0] == '\0') {
    setenv("DISPLAY", ":0", 1);
  }
  int status = system(
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
}

int xkeysnail_stop() {
  int status = system("killall xkeysnail");
  return ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                               : WEXITSTATUS(status));
}

int xkeysnail_status() {
  int status = system("pgrep -x xkeysnail >/dev/null 2>&1");
  status = ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                                 : WEXITSTATUS(status));
}

int command_output(const char* command, char* output, size_t output_size) {
  FILE* fp = popen(command, "r");
  if (NULL == fp) {
    perror("popen");
    return errno;
  }
  if (output && output_size > 0) {
    memset(output, '\0', output_size);
    fgets(output, output_size, fp);
  }
  int status = pclose(fp);
  status = ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                                 : WEXITSTATUS(status));
  return status;
}

void release_modifier_keys() {
  // xorg - How to force release of a keyboard modifiers - Unix & Linux Stack
  // Exchange
  // https://unix.stackexchange.com/questions/60007/how-to-force-release-of-a-keyboard-modifiers
  const char* display = getenv("DISPLAY");
  if (display == NULL || display[0] == '\0') {
    setenv("DISPLAY", ":0", 1);
  }

  command_output(
      "grep '^#define' /usr/include/X11/keysymdef.h | sed -r 's/^#define "
      "XK_(\\S*?).*$/\\1/;' | grep -E '_(L|R|Level.*)$' | xargs xdotool keyup "
      ">/dev/null 2>&1",
      NULL, 0);
}

void xkeysnail_status_maintain() {
  char tty_graphic[3] = {'\0'};
  char tty_active[3] = {'\0'};
  command_output("~/bin/tty-ctl graphic", tty_graphic, sizeof(tty_graphic));
  command_output("~/bin/tty-ctl active", tty_active, sizeof(tty_active));
  if (strcmp(tty_graphic, tty_active) == 0) {
    // Startup xkeysnail instantly after switch to X desktop may freeze desktop,
    // delay startup xkeysnail for rescue.
    usleep(200 * 1000);
  }

  command_output("~/bin/tty-ctl graphic", tty_graphic, sizeof(tty_graphic));
  command_output("~/bin/tty-ctl active", tty_active, sizeof(tty_active));
  if (strcmp(tty_graphic, tty_active) == 0) {
    // Stop keymap if graphic tty active.
    if (command_output("~/bin/keymap-ctl status", NULL, 0) == 0) {
      command_output("~/bin/keymap-ctl stop", NULL, 0);
      fprintf(stderr, "Stop keymap\n");
    }

    // Start xkeysnail if graphic tty active.
    if (xkeysnail_status() != 0) {
      xkeysnail_start();
      fprintf(stderr, "Start xkeysnail\n");
    }

    release_modifier_keys();
  } else {
    release_modifier_keys();

    // Stop xkeysnail if console tty active.
    if (xkeysnail_status() == 0) {
      xkeysnail_stop();
      fprintf(stderr, "Stop xkeysnail\n");
    }

    // Start keymap if graphic tty active.
    if (command_output("~/bin/keymap-ctl status", NULL, 0) != 0) {
      command_output("~/bin/keymap-ctl start", NULL, 0);
      fprintf(stderr, "Start keymap\n");
    }
  }

  // Update polybar xkeysnail icon and set CapsLock off.
  command_output(
      "polybar-msg hook xkeysnail 1 >/dev/null 2>&1; ~/bin/tty-ctl capslockoff",
      NULL, 0);
}

int xkeysnail_monitor() {
  const char* lock_file = "/tmp/xkeysnail-monitor.lock";
  const int lock_fd = open(lock_file, O_CREAT | O_RDWR | O_CLOEXEC, 0666);
  if (lock_fd < 0) {
    fprintf(stderr, "Create lock %s failed: %d\n", lock_file, errno);
    return errno;
  }
  if (flock(lock_fd, LOCK_EX | LOCK_NB) == -1) {
    fprintf(stderr, "Acquire lock %s failed: %d\n", lock_file, errno);
    return errno;
  }

  xkeysnail_status_maintain();

  const int inotify_fd = inotify_init();
  if (inotify_fd == -1) {
    perror("inotify_init");
    return errno;
  }

  int watch_fd =
      inotify_add_watch(inotify_fd, "/sys/class/tty/tty0/active", IN_MODIFY);
  if (watch_fd == -1) {
    perror("inotify_add_watch");
    return errno;
  }

  fprintf(stderr,
          "inotify watch on /sys/class/tty/tty0/active with watch fd %d\n",
          watch_fd);

  watch_fd =
      inotify_add_watch(inotify_fd, "/tmp/xkeysnail.log", IN_CLOSE_WRITE);
  if (watch_fd == -1) {
    perror("inotify_add_watch");
    return errno;
  }

  fprintf(stderr, "inotify watch on /tmp/xkeysnail.log with watch fd %d\n",
          watch_fd);

  char buffer[10 * (sizeof(struct inotify_event) + NAME_MAX + 1)]
      __attribute__((aligned(8)));
  memset(buffer, 0, sizeof(buffer));

  while (1) {
    ssize_t readed = read(inotify_fd, buffer, sizeof(buffer));
    if (readed == -1) {
      perror("read");
      return errno;
    }

    for (char* p = buffer; p < buffer + readed;) {
      struct inotify_event* event = (struct inotify_event*)p;
      fprintf(stderr, "readed inotify event on watch fd %d\n", event->wd);
      p += sizeof(struct inotify_event) + event->len;
    }

    xkeysnail_status_maintain();
  }

  return 0;
}

int main(int argc, char* argv[]) {
  if (argc != 2 ||
      (strcmp(argv[1], "start") != 0 && strcmp(argv[1], "stop") != 0 &&
       strcmp(argv[1], "status") != 0 && strcmp(argv[1], "monitor") != 0)) {
    fprintf(stderr, "Usage: %s <start|stop|status|monitor>\n", argv[0]);
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

  if (strcmp(argv[1], "start") == 0) {
    return xkeysnail_start();
  } else if (strcmp(argv[1], "stop") == 0) {
    return xkeysnail_stop();
  } else if (strcmp(argv[1], "status") == 0) {
    const int status = xkeysnail_status();
    printf("%s\n", (status == 0 ? "started" : "stoped"));
    return status;
  }

  return xkeysnail_monitor();
}
