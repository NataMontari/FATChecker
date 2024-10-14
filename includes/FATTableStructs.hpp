//
// Created by Kateryna on 09.10.2024.
//

#ifndef FATCHECKER_FATTABLESTRUCTS_HPP
#define FATCHECKER_FATTABLESTRUCTS_HPP

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



#endif //FATCHECKER_FATTABLESTRUCTS_HPP
