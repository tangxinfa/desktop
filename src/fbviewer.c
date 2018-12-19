#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr,
            "Usage: %s <file>\n",
            argv[0]);
    return EXIT_FAILURE;
  }

  if (setuid(0) == -1) {
    perror("setuid");
    return EXIT_FAILURE;
  }

  const char* program = "mplayer -vo fbdev2 -msglevel all=0 -fs";
  const char* file = argv[1];
  const char* extension = strrchr(file, '.');
  const int length = strlen(file);

  if (extension != NULL) {
    if (strcasecmp(extension, ".pdf") == 0) {
      program = "fbpdf2";
    } else if (strcasecmp(extension, ".jpg") == 0 ||
               strcasecmp(extension, ".jpeg") == 0 ||
               strcasecmp(extension, ".ppm") == 0 ||
               strcasecmp(extension, ".gif") == 0 ||
               strcasecmp(extension, ".tiff") == 0 ||
               strcasecmp(extension, ".xwd") == 0 ||
               strcasecmp(extension, ".bmp") == 0 ||
               strcasecmp(extension, ".png") == 0 ||
               strcasecmp(extension, ".webp") == 0) {
      program = "fbi -a";
    }
  }

  char command[4096] = {'\0'};
  snprintf(command, sizeof(command), "openvt -s -w -- %s '%s'", program, file);
  int status = system(command);
  status = (status == -1 ? EXIT_FAILURE : WEXITSTATUS(status));
  return status;
}
