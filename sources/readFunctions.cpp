
#include "../includes/readFunctions.hpp"


// bool readBackupBootSector(uint8_t *bootSector, int bootSector_size) {
// #ifdef DEBUG_PRNT
//     std::cout << "Size of boot Sector: " << bootSector_size << std::endl;
//     std::cout << "---------------------------------" << std::endl;
//     std::cout << "Trying to read boot sector" << std::endl;
// #endif
//
//     // Переміщення в початок файлу
//     if (fseek(fp, 512, SEEK_SET) != 0) {
//         perror("Failed to seek boot sector");
//         return false;
//     }
//
//     // Зчитування завантажувального сектора
//     size_t bytesRead = fread(bootSector, 1, bootSector_size, fp);
//     if (bytesRead != bootSector_size) {
//         perror("Failed to read boot sector");
//         return false;
//     }
//
// #ifdef DEBUG_PRNT
//     std::cout << "Successfully read boot sector" << std::endl;
//     std::cout << "Boot Sector Hex Dump:" << std::endl;
//     for (int i = 0; i < bootSector_size; i++) {
//         std::cout << std::hex << (int)bootSector[i] << " ";
//         if ((i + 1) % 16 == 0) std::cout << std::endl; // Вивід через кожні 16 байт
//     }
//     std::cout << std::dec;
// #endif
//
//     return true;
// }
bool readBackupBootSector(uint8_t* bootSector, int bootSector_size) {
    constexpr size_t sectorSize = 512; // Size of one sector
    uint8_t sector[sectorSize];


    // Move to the beginning of the file
    if (fseek(fp, 512, SEEK_SET) != 0) {
        perror("Failed to seek to the beginning of the reserved sectors area");
        return false;
    }

    // Iterate through the file sector by sector
    size_t sectorIndex = 1;
    while (true) {
        // Read the current sector
        size_t bytesRead = fread(sector, 1, sectorSize, fp);
        if (bytesRead != sectorSize) {
            // If we reach the end of the file, exit the loop
            if (feof(fp)) {
                std::cerr << "End of file reached, boot sector pattern not found." << std::endl;
                return false;
            } else {
                perror("Failed to read sector");
                return false;
            }
        }

        // Check for the known boot sector pattern (e.g., the 0x55AA signature at the end)
        if (sector[sectorSize - 2] == 0x55 && sector[sectorSize - 1] == 0xAA) {
            std::cout << "Found boot sector pattern at sector " << sectorIndex << "." << std::endl;

            // If the pattern matches, copy the content to the boot sector
            memcpy(bootSector, sector, bootSector_size);


            return true;
        }

        // Increment the sector index and continue to the next sector
        sectorIndex++;
    }
    if (fseek(fp, 512, SEEK_SET) != 0) {
        perror("Failed to seek to the beginning of the reserved sectors area");
        return false;
    }

}


bool writeBootSectorToFile(const extFAT12_16* bpb) {
    constexpr int bootSectorSize = sizeof(extFAT12_16);
    std::cout<<"Copying the backup boot sector to the original boot sector..."<<std::endl;
#ifdef DEBUG_PRNT
    std::cout << "Size of boot sector: " << bootSectorSize << std::endl;
    std::cout << "---------------------------------" << std::endl;
    std::cout << "Trying to write boot sector" << std::endl;
#endif

    // Move to the start of the boot sector (offset 0)
    if (fseek(fp, 0, SEEK_SET) != 0) {
        perror("Failed to seek to boot sector in file");
        return false;
    }

    // Write the boot sector to the file
    size_t bytesWritten = fwrite(bpb, 1, bootSectorSize, fp);
    if (bytesWritten != bootSectorSize) {
        perror("Failed to write boot sector");
        return false;
    }
    std::cout << "Successfully written boot sector" << std::endl;
    return true;
}

void writeFATTableToFile(uint16_t* FAT, int FATIndex, uint16_t bytesPerSec, int FATSize, int startFATSector) {
    // Розрахувати зміщення до початку FAT таблиці
    size_t offset = startFATSector * bytesPerSec + FATIndex * FATSize * bytesPerSec;

    // Перемістити курсор у файл
    if (fseek(fp, offset, SEEK_SET) != 0) {
        std::cerr << "Error seeking to FAT table " << FATIndex + 1 << " in file!" << std::endl;
        return;
    }

    // Записати FAT таблицю у файл
    size_t written = fwrite(FAT, sizeof(uint16_t), FATSize * bytesPerSec / sizeof(uint16_t), fp);

    if (written != FATSize * bytesPerSec / sizeof(uint16_t)) {
        std::cerr << "Error writing FAT table " << FATIndex + 1 << " to file!" << std::endl;
    } else {
        std::cout << "FAT table " << FATIndex + 1 << " has been successfully written to the file." << std::endl;
    }
}
