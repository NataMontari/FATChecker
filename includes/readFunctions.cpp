
#include "readFunctions.hpp"
bool readBackupBootSector(uint8_t *bootSector, int bootSector_size) {
#ifdef DEBUG_PRNT
    std::cout << "Size of boot Sector: " << bootSector_size << std::endl;
    std::cout << "---------------------------------" << std::endl;
    std::cout << "Trying to read boot sector" << std::endl;
#endif

    // Переміщення в початок файлу
    if (fseek(fp, 512, SEEK_SET) != 0) {
        perror("Failed to seek boot sector");
        return false;
    }

    // Зчитування завантажувального сектора
    size_t bytesRead = fread(bootSector, 1, bootSector_size, fp);
    if (bytesRead != bootSector_size) {
        perror("Failed to read boot sector");
        return false;
    }

#ifdef DEBUG_PRNT
    std::cout << "Successfully read boot sector" << std::endl;
    std::cout << "Boot Sector Hex Dump:" << std::endl;
    for (int i = 0; i < bootSector_size; i++) {
        std::cout << std::hex << (int)bootSector[i] << " ";
        if ((i + 1) % 16 == 0) std::cout << std::endl; // Вивід через кожні 16 байт
    }
    std::cout << std::dec;
#endif

    return true;
}