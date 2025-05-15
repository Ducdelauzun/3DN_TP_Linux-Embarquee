/*******************************************************
 * Fichier : led3.c
 * Rôle    : Allumer la LED numéro 3 via accès mémoire mmap
 * Auteur  : Gad Katamba
 * Date    : 15 mai 2025
 ******************************************************/

 #include <stdio.h>
 #include <stdlib.h>
 #include <fcntl.h>
 #include <sys/mman.h>
 #include <unistd.h>
 #include <stdint.h>
 
 #define HW_REGS_BASE     0xFF203000
 #define HW_REGS_SPAN     0x20         // On mappe une petite zone mémoire
 #define GPIO_DATA_OFFSET 0x0          // Offset registre DATA
 #define GPIO_DIR_OFFSET  0x4          // Offset registre DIRECTION
 
 int main() {
     int fd;
     void* gpio_map;
     volatile uint32_t* gpio;
 
     // Ouverture du périphérique mémoire
     fd = open("/dev/mem", O_RDWR | O_SYNC);
     if (fd < 0) {
         perror("Erreur ouverture /dev/mem");
         return EXIT_FAILURE;
     }
 
     // Mapping de la mémoire physique dans l’espace utilisateur
     gpio_map = mmap(NULL, HW_REGS_SPAN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, HW_REGS_BASE);
     if (gpio_map == MAP_FAILED) {
         perror("Erreur mmap");
         close(fd);
         return EXIT_FAILURE;
     }
 
     gpio = (volatile uint32_t*) gpio_map;
 
     // Configuration de la LED 3 (bit 3) en sortie
     gpio[GPIO_DIR_OFFSET / 4] |= (1 << 3);  // met le bit 3 du registre DIRECTION à 1
 
     // Allumage de la LED 3 (bit 3 du registre DATA)
     gpio[GPIO_DATA_OFFSET / 4] = (1 << 3);  // 0x08
 
     printf("LED 3 allumée.\n");
 
     // Pause pour laisser la LED allumée un instant (optionnel)
     sleep(2);
 
     // Libération des ressources
     munmap(gpio_map, HW_REGS_SPAN);
     close(fd);
 
     return EXIT_SUCCESS;
 } 