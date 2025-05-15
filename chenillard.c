#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define NUM_LEDS 10
#define PATH_TEMPLATE "/sys/class/leds/fpga_led%d/brightness"
#define SLEEP_US 200000  

void set_led(int index, int state) {
    char path[100];
    snprintf(path, sizeof(path), PATH_TEMPLATE, index);

    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror(path);
        return;
    }

    if (write(fd, state ? "1" : "0", 1) < 0) {
        perror("write");
    }

    close(fd);
}

void clear_all_leds() {
    for (int i = 1; i < NUM_LEDS; ++i) {
        set_led(i, 0);
    }
}

int main() {
    int direction = 1;  
    int pos = 0;

    while (1) {
        clear_all_leds();
        set_led(pos, 1);
        usleep(SLEEP_US);

        pos += direction;

        if (pos == NUM_LEDS - 1 || pos == 0) {
            direction *= -1;
        }
    }

    return 0;
}