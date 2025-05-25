#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define GPIO_BASE_ADDR   0xFF203000
#define MAP_SIZE         0x1000  // 4 Ko (taille minimale d'une page mémoire)

int main() {
    int fd;
    void *virtual_base;
    volatile uint32_t *gpio_ptr;

    // Ouvre l'accès à la mémoire physique
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        perror("Erreur ouverture /dev/mem");
        return 1;
    }

    // Mappe la mémoire à partir de l'adresse GPIO_BASE_ADDR
    virtual_base = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_BASE_ADDR);
    if (virtual_base == MAP_FAILED) {
        perror("Erreur mmap");
        close(fd);
        return 1;
    }

    // Cast du pointeur vers le registre GPIO
    gpio_ptr = (volatile uint32_t *)virtual_base;

    // Chenillard simple : allume les LED une par une
    for (int i = 0; i < 8; i++) {
        *gpio_ptr = (1 << i);
        usleep(200000); // 200 ms
    }

    // Éteint les LED
    *gpio_ptr = 0;

    // Libère la mémoire et ferme /dev/mem
    munmap(virtual_base, MAP_SIZE);
    close(fd);

    return 0;
}
