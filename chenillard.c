#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define LED_COUNT 10
#define LED_PATH_FORMAT "/sys/class/leds/fpga_led%d/brightness"
#define DELAY_US 200000

void set_led(int led_num, int value) {
    char path[64];
    int fd;
    snprintf(path, sizeof(path), LED_PATH_FORMAT, led_num);
    fd = open(path, O_WRONLY);
    if (fd < 0) return;
    if (value)
        write(fd, "1", 1);
    else
        write(fd, "0", 1);
    close(fd);
}

int main() {
    int i, j;
    while (1) {
        for (i = 1; i <= LED_COUNT; i++) {
            for (j = 1; j <= LED_COUNT; j++)
                set_led(j, j == i ? 1 : 0);
            usleep(DELAY_US);
        }
        for (i = LED_COUNT; i >= 1; i--) {
            for (j = 1; j <= LED_COUNT; j++)
                set_led(j, j == i ? 1 : 0);
            usleep(DELAY_US);
        }
    }
    return 0;
}