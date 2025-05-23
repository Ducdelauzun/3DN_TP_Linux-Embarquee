/************************************************************
 * Fichier : clock_7seg.c
 * Rôle    : Affiche l'heure sur les afficheurs 7 segments (VEEK)
 * Auteur  : Gad Katamba
 * Date    : 15 mai 2025
 ************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>

// FPGA memory mapping constants
#define FPGA_HEX_BASE       0x33000
#define ALT_LWFPGASLVS_OFST 0x00000000
#define HW_REGS_BASE        0xFF200000
#define HW_REGS_SPAN        0x00005000
#define HW_REGS_MASK        (HW_REGS_SPAN - 1)

// 7-segment display patterns (0-9)
static const uint8_t SEG_PATTERNS[] = {
    63, 6, 91, 79, 102, 109, 125, 7, 127, 111  // 0-9
};

int main() {
    int fd;
    void *virtual_base;
    uint8_t *hex_base;
    uint8_t patterns[] = {0xFF, 0x00, 0xAA, 0x55};
    int i = 0;

    // Open /dev/mem
    fd = open("/dev/mem", (O_RDWR | O_SYNC));
    if (fd == -1) {
        perror("Failed to open /dev/mem");
        return -1;
    }

    // Map the FPGA memory space
    virtual_base = mmap(NULL, HW_REGS_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, HW_REGS_BASE);
    if (virtual_base == MAP_FAILED) {
        perror("Failed to mmap");
        close(fd);
        return -1;
    }

    // Calculate the base address for the 7-segment displays
    hex_base = (uint8_t *)virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + FPGA_HEX_BASE) & (unsigned long)(HW_REGS_MASK));

    printf("Test d'écriture brute sur le premier afficheur HEX (uint8_t). Ctrl+C pour quitter.\n");
    while (1) {
        *((uint8_t *)hex_base) = patterns[i % 4];
        printf("Ecrit 0x%02X sur l'adresse HEX\n", patterns[i % 4]);
        i++;
        sleep(1);
    }

    munmap(virtual_base, HW_REGS_SPAN);
    close(fd);
    return 0;
}
