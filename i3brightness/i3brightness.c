/**
 * @file   i3brightness.c
 * @author tangxinfa <tangxinfa@xunlei.com>
 * @date   Fri Jan 20 15:14:38 2017
 *
 * @brief  Screen brightness control.
 *
 * @link https://www.reddit.com/r/linux/comments/268wcy/managing_brightness_on_i3wm/ @endlink
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


#ifndef BACKLIGHT
#define BACKLIGHT "intel_backlight"
#endif

#ifndef MIN_BRIGHTNESS_PERCENT
#define MIN_BRIGHTNESS_PERCENT 20
#endif

int str_to_value (const char* str, uint32_t* value) {
    char* end = NULL;
    *value = strtoul(str, &end, 10);
    if (end && (end[0] == '\0' || end[0] == '\n')) {
        return 0;
    }

    return -1;
}

int read_value(const char* file, uint32_t* value) {
    int fd = open(file, O_RDONLY);
    if (fd == -1) {
        return -1;
    }

    char data[32] = {'\0'};
    if (-1 == read(fd, data, sizeof(data) - 1)) {
        close(fd);
        return -1;
    }

    close(fd);

    return str_to_value(data, value);
}

int write_value(const char* file, uint32_t value) {
    int fd = open(file, O_WRONLY|O_TRUNC);
    if (fd == -1) {
        return -1;
    }

    char data[32] = {'\0'};
    snprintf(data, sizeof(data), "%"PRIu32, value);

    if (-1 == write(fd, data, strlen(data))) {
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

const char* brightness_file = "/sys/class/backlight/"BACKLIGHT"/brightness";
const char* max_brightness_file = "/sys/class/backlight/"BACKLIGHT"/max_brightness";

int main(int argc, char *argv[])
{
    if (argc != 2 || strcmp(argv[1], "-h") == 0) {
        fprintf(stderr,
                "usage: %s <option>\n\n"
                "option:\n"
                "    +<d> Increase brightness and output new brightness, for example: +500 or +%5\n"
                "    -<d> Decrease brightness and output new brightness, for example: -500 or -%5\n"
                "    -b   Output brightness\n"
                "    -B   Output brightness percent\n"
                "    -m   Output max brightness\n"
                "    -n   Output min brightness\n"
                "    -N   Output min brightness percent\n"
                "    -h   Show this help message\n",
                argv[0]);
        return 1;
    }

    uint32_t max_brightness;
    if (-1 == read_value(max_brightness_file, &max_brightness)) {
        fprintf(stderr, "read %s error: %s\n", max_brightness_file, strerror(errno));
        return 1;
    }

    uint32_t min_brightness = MIN_BRIGHTNESS_PERCENT*(max_brightness/100);

    uint32_t brightness;
    if (-1 == read_value(brightness_file, &brightness)) {
        fprintf(stderr, "read %s error: %s\n", brightness_file, strerror(errno));
        return 1;
    }

    uint32_t value;

    if (strcmp(argv[1], "-b") == 0) {
        printf("%"PRIu32"\n", brightness);
        return 0;
    } else if (strcmp(argv[1], "-B") == 0) {
        printf("%"PRIu32"\n", brightness*100/max_brightness);
        return 0;
    } else if (strcmp(argv[1], "-m") == 0) {
        printf("%"PRIu32"\n", max_brightness);
        return 0;
    } else if (strcmp(argv[1], "-n") == 0) {
        printf("%"PRIu32"\n", min_brightness);
        return 0;
    } else if (strcmp(argv[1], "-N") == 0) {
        printf("%"PRIu32"\n", MIN_BRIGHTNESS_PERCENT);
        return 0;
    } else if (argv[1][0] == '+') {
        if (0 != str_to_value(&argv[1][argv[1][1] == '%' ? 2 : 1], &value)) {
            fprintf(stderr, "invalid option: %s\n", argv[1]);
            return 1;
        }

        if (argv[1][1] == '%') {
            if (value > 100) {
                value = 100;
            } else if (value < 0) {
                value = 0;
            }
            value = max_brightness*value/100;
        }

        if(brightness + value > max_brightness) {
            brightness = max_brightness;
        } else {
            brightness += value;
        }

        if (brightness < min_brightness) {
            brightness = min_brightness;
        }

        if (-1 == write_value(brightness_file, brightness)) {
            fprintf(stderr, "write %s error: %s\n", brightness_file, strerror(errno));
            return 1;
        }
        printf("%"PRIu32"\n", argv[1][1] == '%' ? brightness*100/max_brightness : brightness);
        return 0;
    } else if (argv[1][0] == '-') {
        if (0 != str_to_value(&argv[1][argv[1][1] == '%' ? 2 : 1], &value)) {
            fprintf(stderr, "invalid option: %s\n", argv[1]);
            return 1;
        }

        if (argv[1][1] == '%') {
            if (value > 100) {
                value = 100;
            } else if (value < 0) {
                value = 0;
            }
            value = max_brightness*value/100;
        }

        if(value > brightness) {
            brightness = 0;
        } else {
            brightness -= value;
        }

        if (brightness < min_brightness) {
            brightness = min_brightness;
        }

        if (-1 == write_value(brightness_file, brightness)) {
            fprintf(stderr, "write %s error: %s\n", brightness_file, strerror(errno));
            return 1;
        }
        printf("%"PRIu32"\n", argv[1][1] == '%' ? brightness*100/max_brightness : brightness);
        return 0;
    }

    fprintf(stderr, "invalid option: %s\n", argv[1]);
    return 1;
}
