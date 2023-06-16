#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/inotify.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int command_output(const char* command, char* output, size_t output_size) {
  FILE* fp = popen(command, "r");
  if (NULL == fp) {
    perror("popen");
    return EXIT_FAILURE;
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

int attach_graphic_display() {
  char display[32] = {'\0'};
  command_output("~/bin/tty-ctl graphic-display", display, sizeof(display));
  char* eol = strchr(display, '\n');
  if (eol) {
    *eol = '\0';
  }
  if (display[0] == '\0') {
    fprintf(stderr, "Graphic display not found\n");
    return EXIT_FAILURE;
  }
  setenv("DISPLAY", display, 1);
  return 0;
}

int xkeysnail_start() {
  int status = attach_graphic_display();
  if (status != 0) {
    return status;
  }
  status = system(
      "xhost +SI:localuser:root >/dev/stderr; "
      "xkeysnail ~/.config/xkeysnail/config.py --quiet --watch "
      ">/dev/stderr &");
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

static int g_notify_fd = -1;

void notify(int sig) {
  char c = '\0';
  write(g_notify_fd, &c, 1);
}

void* xkeysnail_run(void* param) {
  int status = attach_graphic_display();
  if (status != 0) {
    return NULL;
  }
  system(
      "xhost +SI:localuser:root >/dev/stderr; "
      "exec xkeysnail ~/.config/xkeysnail/config.py --quiet --watch "
      ">/dev/stderr");

  notify(SIGCHLD);

  return NULL;
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

int release_modifier_keys() {
  // xorg - How to force release of a keyboard modifiers - Unix & Linux Stack
  // Exchange
  // https://unix.stackexchange.com/questions/60007/how-to-force-release-of-a-keyboard-modifiers
  int status = attach_graphic_display();
  if (status != 0) {
    return status;
  }

  return command_output(
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
      pthread_t tid;
      pthread_attr_t attr;
      if (0 != pthread_attr_init(&attr)) {
        perror("pthread_attr_init");
        exit(EXIT_FAILURE);
      }
      if (0 != pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) {
        perror("pthread_attr_setdetachstate");
        exit(EXIT_FAILURE);
      }
      if (0 != pthread_create(&tid, &attr, &xkeysnail_run, NULL)) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
      }
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
    return EXIT_FAILURE;
  }
  if (flock(lock_fd, LOCK_EX | LOCK_NB) == -1) {
    if (errno == EWOULDBLOCK) {
      fprintf(stderr,
              "Acquire lock %s failed: %d, maybe a instance already running, "
              "try to wake it up!\n",
              lock_file, errno);
      system("killall -USR1 xkeysnaild");
    } else {
      fprintf(stderr, "Acquire lock %s failed: %d\n", lock_file, errno);
    }
    return EXIT_FAILURE;
  }

  int pipe_fd[2] = {-1, -1};
  if (-1 == pipe(pipe_fd)) {
    perror("pipe");
    return EXIT_FAILURE;
  }

  if (fcntl(pipe_fd[0], F_SETFL, O_NONBLOCK) < 0) {
    perror("fcntl");
    return EXIT_FAILURE;
  }

  g_notify_fd = pipe_fd[1];

  prctl(PR_SET_NAME, "xkeysnaild");

  if (SIG_ERR == signal(SIGUSR1, notify)) {
    perror("signal");
    return EXIT_FAILURE;
  }

  xkeysnail_stop();
  xkeysnail_status_maintain();

  const int inotify_fd = inotify_init();
  if (inotify_fd == -1) {
    perror("inotify_init");
    return EXIT_FAILURE;
  }

  int watch_fd =
      inotify_add_watch(inotify_fd, "/sys/class/tty/tty0/active", IN_MODIFY);
  if (watch_fd == -1) {
    perror("inotify_add_watch /sys/class/tty/tty0/active");
    return EXIT_FAILURE;
  }

  fprintf(stderr,
          "inotify watch on /sys/class/tty/tty0/active with watch fd %d\n",
          watch_fd);

  char buffer[10 * (sizeof(struct inotify_event) + NAME_MAX + 1)]
      __attribute__((aligned(8)));
  memset(buffer, 0, sizeof(buffer));
  fd_set fdset;

  while (1) {
    FD_ZERO(&fdset);
    FD_SET(inotify_fd, &fdset);
    FD_SET(pipe_fd[0], &fdset);

    if (select((inotify_fd > pipe_fd[0] ? inotify_fd : pipe_fd[0]) + 1, &fdset,
               NULL, NULL, NULL) == -1) {
      if (errno != EINTR) {
        perror("select");
        return EXIT_FAILURE;
      }
      continue;
    }

    if (FD_ISSET(inotify_fd, &fdset)) {
      read(inotify_fd, buffer, sizeof(buffer));
    }

    if (FD_ISSET(pipe_fd[0], &fdset)) {
      // Read all data until empty.
      while (read(pipe_fd[0], buffer, sizeof(buffer)) > 0) {
      }
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

  signal(SIGTERM, exit);
  return xkeysnail_monitor();
}
