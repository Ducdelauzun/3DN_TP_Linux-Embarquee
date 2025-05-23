#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

// Adresses issues de l'exemple
#define FPGA_HEX_BASE       0x33000
#define ALT_LWFPGASLVS_OFST 0x00000000
#define HW_REGS_BASE        0xFF200000
#define HW_REGS_SPAN        0x00005000
#define HW_REGS_MASK        (HW_REGS_SPAN - 1)

static int fpga_fd = -1;
static void *fpga_virtual_base = NULL;
static uint8_t *fpga_hex_base = NULL;

static const uint8_t szMask[16] = {
    63, 6, 91, 79, 102, 109, 125, 7,
    127, 111, 119, 124, 57, 94, 121, 113
};

bool fpga_init() {
    fpga_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fpga_fd < 0) {
        perror("open /dev/mem");
        return false;
    }
    fpga_virtual_base = mmap(NULL, HW_REGS_SPAN, PROT_READ | PROT_WRITE, MAP_SHARED, fpga_fd, HW_REGS_BASE);
    if (fpga_virtual_base == MAP_FAILED) {
        perror("mmap");
        close(fpga_fd);
        fpga_fd = -1;
        return false;
    }
    fpga_hex_base = (uint8_t *)fpga_virtual_base + ((ALT_LWFPGASLVS_OFST + FPGA_HEX_BASE) & HW_REGS_MASK);
    return true;
}

void fpga_cleanup() {
    if (fpga_virtual_base && fpga_virtual_base != MAP_FAILED) {
        munmap(fpga_virtual_base, HW_REGS_SPAN);
        fpga_virtual_base = NULL;
    }
    if (fpga_fd >= 0) {
        close(fpga_fd);
        fpga_fd = -1;
    }
}

// index: 0 à 5, value: 0 à 15 (0-9, A-F)
void fpga_hex_set(int index, int value) {
    if (!fpga_hex_base) return;
    if (index < 0 || index > 5) return;
    if (value < 0) value = 0;
    if (value > 15) value = 15;
    *((uint32_t *)fpga_hex_base + index) = szMask[value];
} 