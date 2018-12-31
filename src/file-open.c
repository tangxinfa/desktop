#include <limits.h>
#include <linux/limits.h>  // for PATH_MAX
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int console_open(const char* file) {
  const char* program = "mplayer -vo fbdev2 -msglevel all=0 -fs";
  const char* extension = strrchr(file, '.');
  const int length = strlen(file);

  if (extension == NULL) {
    return EXIT_FAILURE;
  }
  if (strcasecmp(extension, ".xls") == 0 ||
      strcasecmp(extension, ".xlsx") == 0 ||
      strcasecmp(extension, ".doc") == 0 ||
      strcasecmp(extension, ".docx") == 0 ||
      strcasecmp(extension, ".ppt") == 0 ||
      strcasecmp(extension, ".pptx") == 0) {
    return EXIT_FAILURE;
  } else if (strcasecmp(extension, ".pdf") == 0) {
    program = "fbpdf2";
  } else if (strcasecmp(extension, ".jpg") == 0 ||
             strcasecmp(extension, ".jpeg") == 0 ||
             strcasecmp(extension, ".ppm") == 0 ||
             strcasecmp(extension, ".tiff") == 0 ||
             strcasecmp(extension, ".xwd") == 0 ||
             strcasecmp(extension, ".bmp") == 0 ||
             strcasecmp(extension, ".png") == 0 ||
             strcasecmp(extension, ".webp") == 0) {
    program = "fbi -a";
  } else if (strcasecmp(extension, ".gif") == 0) {
    program = "mplayer -vo fbdev2 -msglevel all=0 -fs -loop 0";
  }

  char command[4096] = {'\0'};
  snprintf(command, sizeof(command), "openvt -s -w -- %s '%s'", program, file);
  int status = system(command);
  status = ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                                 : WEXITSTATUS(status));

  return status;
}

int graphic_open(const char* file) {
  char command[4096] = {'\0'};
  const char* display = getenv("DISPLAY");
  if (display == NULL || display[0] == '\0') {
    system("~/bin/tty-ctl other");
    display = ":0";
  }

  snprintf(command, sizeof(command),
           "DISPLAY=%s xdg-open '%s' >/dev/null 2>&1 &", display, file);
  int status = system(command);
  status = ((status == -1 || !WIFEXITED(status)) ? EXIT_FAILURE
                                                 : WEXITSTATUS(status));
  return status;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <file>\n", argv[0]);
    return EXIT_FAILURE;
  }

  char file[PATH_MAX] = {'\0'};
  if (NULL == realpath(argv[1], file)) {
    fprintf(stderr, "Error: file \"%s\" not exists\n", argv[1]);
    return EXIT_FAILURE;
  }

  const char* display = getenv("DISPLAY");
  if (display && display[0] != '\0') {
    return graphic_open(file);
  }

  const uid_t uid = getuid();
  if (setuid(0) == -1) {
    perror("setuid");
    return EXIT_FAILURE;
  }

  int status = console_open(file);
  if (status != EXIT_SUCCESS) {
    status = setuid(uid);
    status = graphic_open(file);
  }

  return status;
}
