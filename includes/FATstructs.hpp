
#ifndef FATCHECKER_FATSTRUCTS_HPP
#define FATCHECKER_FATSTRUCTS_HPP

#include <cstdint>

#pragma pack(push, 1)  // Забезпечує відсутність вирівнювання полів у структурі

typedef struct {
    uint8_t BS_jmpBoot[3];          // Перші 3 байти — це код переходу
    char BS_OEMName[8];              // Ідентифікатор MS
    uint16_t BPB_BytsPerSec;       // Кількість байтів на сектор
    uint8_t BPB_SecPerClus;     // Кількість секторів на кластер
    uint16_t BPB_RsvdSecCnt;      // Зарезервовані сектори
    uint8_t BPB_NumFATs;               // Кількість FAT таблиць
    uint16_t BPB_RootEntCnt;       // Кількість записів у кореневому каталозі
    uint16_t BPB_TotSec16;       // Загальна кількість секторів (якщо < 65536)
    uint8_t BPB_Media;             // Тип носія
    uint16_t BPB_FATSz16;            // Розмір FAT в секторах (для FAT12/FAT16)
    uint16_t BPB_SecPerTrk;      // Кількість секторів на доріжку
    uint16_t BPB_NumHeads;             // Кількість головок
    uint32_t BPB_HiddSec;        // Приховані сектори
    uint32_t BPB_TotSec32;       // Загальна кількість секторів (якщо більше 65536)
} BasicFAT;


typedef struct {
    BasicFAT basic;                 // Вкладена структура для базових полів FAT
    uint8_t BS_DrvNum;              // Номер диска
    uint8_t BS_Reserved1;           // Зарезервовано
    uint8_t BS_BootSig;             // Завантажувальний підпис
    uint32_t BS_VolID;              // Серійний номер тому
    char BS_VolLab[11];             // Мітка тому
    char BS_FilSysType[8];          // Тип файлової системи ("FAT12   " або "FAT16   ")
} extFAT12_16;

typedef struct {
    BasicFAT basic;                 // Вкладена структура для базових полів FAT

    // Специфічні для FAT32 поля
    uint32_t BPB_FATSz32;           // Розмір FAT в секторах (для FAT32)
    uint16_t BPB_ExtFlags;          // Розширені флаги
    uint16_t BPB_FSVer;             // Версія файлової системи
    uint32_t BPB_RootClus;          // Номер першого кластера кореневого каталогу (для FAT32)
    uint16_t BPB_FSInfo;            // Сектор FSInfo
    uint16_t BPB_BkBootSec;         // Зарезервований сектор для копії бут-сектора
    uint8_t BPB_Reserved[12];       // Зарезервовані байти

    // Розширені поля завантажувального сектора
    uint8_t BS_DrvNum;              // Номер диска
    uint8_t BS_Reserved1;           // Зарезервовано
    uint8_t BS_BootSig;             // Завантажувальний підпис
    uint32_t BS_VolID;              // Серійний номер тома
    char BS_VolLab[11];             // Мітка тому
    char BS_FilSysType[8];          // Тип файлової системи ("FAT32   ")

    uint8_t BS_BootCode[420];       // Завантажувальний код
    uint16_t BS_BootSectorSig;      // Підпис бут-сектора (0xAA55)
} extFAT32;

#pragma pack(pop)  // Повертає попереднє вирівнювання

#endif //FATCHECKER_FATSTRUCTS_HPP
