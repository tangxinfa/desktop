#include <limits.h>
#include <linux/limits.h>  // for PATH_MAX
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int console_open(const char* file) {
  const char* program = NULL;
  const char* extension = strrchr(file, '.');
  const int length = strlen(file);

  if (extension == NULL) {
    return EXIT_FAILURE;
  }
  if (strcasecmp(extension, ".pdf") == 0) {
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
  } else if (strcasecmp(extension, ".3g2") == 0 ||
             strcasecmp(extension, ".3gp") == 0 ||
             strcasecmp(extension, ".asf") == 0 ||
             strcasecmp(extension, ".ask") == 0 ||
             strcasecmp(extension, ".avi") == 0 ||
             strcasecmp(extension, ".c3d") == 0 ||
             strcasecmp(extension, ".dat") == 0 ||
             strcasecmp(extension, ".divx") == 0 ||
             strcasecmp(extension, ".dvr-ms") == 0 ||
             strcasecmp(extension, ".f4v") == 0 ||
             strcasecmp(extension, ".flc") == 0 ||
             strcasecmp(extension, ".fli") == 0 ||
             strcasecmp(extension, ".flv") == 0 ||
             strcasecmp(extension, ".flx") == 0 ||
             strcasecmp(extension, ".m2p") == 0 ||
             strcasecmp(extension, ".m2t") == 0 ||
             strcasecmp(extension, ".m2ts") == 0 ||
             strcasecmp(extension, ".m2v") == 0 ||
             strcasecmp(extension, ".m4v") == 0 ||
             strcasecmp(extension, ".mkv") == 0 ||
             strcasecmp(extension, ".mlv") == 0 ||
             strcasecmp(extension, ".mov") == 0 ||
             strcasecmp(extension, ".mp4") == 0 ||
             strcasecmp(extension, ".mpe") == 0 ||
             strcasecmp(extension, ".mpeg") == 0 ||
             strcasecmp(extension, ".mpg") == 0 ||
             strcasecmp(extension, ".mpv") == 0 ||
             strcasecmp(extension, ".mts") == 0 ||
             strcasecmp(extension, ".ogm") == 0 ||
             strcasecmp(extension, ".qt") == 0 ||
             strcasecmp(extension, ".ra") == 0 ||
             strcasecmp(extension, ".rm") == 0 ||
             strcasecmp(extension, ".rmvb") == 0 ||
             strcasecmp(extension, ".swf") == 0 ||
             strcasecmp(extension, ".tp") == 0 ||
             strcasecmp(extension, ".trp") == 0 ||
             strcasecmp(extension, ".ts") == 0 ||
             strcasecmp(extension, ".uis") == 0 ||
             strcasecmp(extension, ".uisx") == 0 ||
             strcasecmp(extension, ".uvp") == 0 ||
             strcasecmp(extension, ".vob") == 0 ||
             strcasecmp(extension, ".vsp") == 0 ||
             strcasecmp(extension, ".webm") == 0 ||
             strcasecmp(extension, ".wmv") == 0 ||
             strcasecmp(extension, ".wmvhd") == 0 ||
             strcasecmp(extension, ".wtv") == 0 ||
             strcasecmp(extension, ".xvid") == 0) {
    program = "mplayer -vo fbdev2 -msglevel all=0 -fs";
  } else {
    return EXIT_FAILURE;
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
  // Fallback to graphic_open if file not support to open in console.
  if (status == EXIT_FAILURE) {
    status = setuid(uid);
    status = graphic_open(file);
  }

  return status;
}
