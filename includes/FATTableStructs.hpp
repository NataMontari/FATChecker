
#ifndef FATCHECKER_FATTABLESTRUCTS_HPP
#define FATCHECKER_FATTABLESTRUCTS_HPP

#include <cstdint>

typedef struct{

} FATTable16;

enum FAT12Entry {
    FAT12_FREE_CLUSTER = 0x000,          // Кластер вільний
    FAT12_ALLOCATED_CLUSTER_MIN = 0x002, // Мін значення для кластеру, що виділений
    FAT12_ALLOCATED_CLUSTER_MAX = 0xFF6, // Макс значення для виділеного кластеру
    FAT12_BAD_CLUSTER = 0xFF7,           // Поганий (дефектний) кластер
    FAT12_RESERVED_CLUSTER_MIN = 0xFF8,  // Мін зарезервоване значення
    FAT12_RESERVED_CLUSTER_MAX = 0xFFE,  // Макс зарезервоване значення
    FAT12_END_OF_FILE = 0xFFF            // Кластер є останнім у файлі (кінець файлу)
};

enum FAT16Entry {
    FAT16_FREE_CLUSTER = 0x0000,          // Кластер вільний
    FAT16_ALLOCATED_CLUSTER_MIN = 0x0002, // Мін значення для кластеру, що виділений
    FAT16_ALLOCATED_CLUSTER_MAX = 0xFFF6, // Макс значення для виділеного кластеру
    FAT16_BAD_CLUSTER = 0xFFF7,           // Поганий (дефектний) кластер
    FAT16_RESERVED_CLUSTER_MIN = 0xFFF8,  // Мін зарезервоване значення
    FAT16_RESERVED_CLUSTER_MAX = 0xFFFE,  // Макс зарезервоване значення
    FAT16_END_OF_FILE = 0xFFFF            // Кластер є останнім у файлі (кінець файлу)
};

enum FAT32Entry {
    FAT32_FREE_CLUSTER = 0x00000000,          // Кластер вільний
    FAT32_ALLOCATED_CLUSTER_MIN = 0x00000002, // Мін значення для кластеру, що виділений
    FAT32_ALLOCATED_CLUSTER_MAX = 0xFFFFFF6,  // Макс значення для виділеного кластеру
    FAT32_BAD_CLUSTER = 0xFFFFFF7,            // Поганий (дефектний) кластер
    FAT32_RESERVED_CLUSTER_MIN = 0xFFFFFF8,   // Мін зарезервоване значення
    FAT32_RESERVED_CLUSTER_MAX = 0xFFFFFFE,   // Макс зарезервоване значення
    FAT32_END_OF_FILE = 0xFFFFFFFF            // Кластер є останнім у файлі (кінець файлу)
};

// Структура для зручного доступу до FAT12-енетрів
struct FAT12 {
    uint8_t* FATData;  // Вказівник на дані FAT
    int FATSize;       // Розмір FAT в байтах

    FAT12(uint8_t* data, int size) : FATData(data), FATSize(size) {}

    // Читання FAT12-ендрі для заданого кластеру
    uint16_t getEntry(int clusterIndex) {
        int byteOffset = clusterIndex * 3 / 2;
        int bitOffset = (clusterIndex & 1) * 4;

        // std::cout<<"Cluster index: "<<clusterIndex<<std::endl;
        if (byteOffset + 1 >= FATSize) { // Перевіряємо межі масиву
            std::cerr << "Error: Byte offset out of range: " << byteOffset << "\n";
            return 0xFFF;
        }

        uint16_t entry = FATData[byteOffset] | (FATData[byteOffset + 1] << 8);
        entry = (entry >> bitOffset) & 0xFFF; // Маскуємо зайві біти для 12-бітного значення

        return entry;
    }

    // Запис значення в FAT12
    void setEntry(int clusterIndex, uint16_t value) {
        int byteOffset = clusterIndex * 3 / 2;
        int bitOffset = (clusterIndex & 1) * 8;

        if (bitOffset == 0) {
            FATData[byteOffset] = value & 0xFF;
            FATData[byteOffset + 1] = (value >> 8) & 0xFF;
        } else {
            FATData[byteOffset] = (value << 4) & 0xFF;
            FATData[byteOffset + 1] = (value >> 4) & 0xFF;
        }
    }
    void freeCluster(int cluster) {
        setEntry(cluster, 0x000); // Позначаємо кластер як вільний
    }
};


#endif //FATCHECKER_FATTABLESTRUCTS_HPP
