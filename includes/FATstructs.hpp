
#ifndef FATCHECKER_FATSTRUCTS_HPP
#define FATCHECKER_FATSTRUCTS_HPP

#include <cstdint>
#include <vector>

#pragma pack(push, 1)  // Забезпечує відсутність вирівнювання полів у структурі

typedef struct {
    uint8_t BS_jmpBoot[3];          // Перші 3 байти — це код переходу
    char BS_OEMName[8];              // Ідентифікатор MS
    uint16_t BPB_BytsPerSec;       // К-сть байтів на сектор
    uint8_t BPB_SecPerClus;     // К-сть секторів на кластер
    uint16_t BPB_RsvdSecCnt;      // Зарезервовані сектори
    uint8_t BPB_NumFATs;               // К-сть FAT таблиць
    uint16_t BPB_RootEntCnt;       // К-сть записів у кореневому каталозі
    uint16_t BPB_TotSec16;       // Загальна к-сть секторів (якщо < 65536)
    uint8_t BPB_Media;             // Тип носія
    uint16_t BPB_FATSz16;            // Розмір FAT в секторах (для FAT12/FAT16)
    uint16_t BPB_SecPerTrk;      // К-сть секторів на доріжку
    uint16_t BPB_NumHeads;             // К-сть головок
    uint32_t BPB_HiddSec;        // Приховані сектори
    uint32_t BPB_TotSec32;       // Загальна к-сть секторів (якщо більше 65536)
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

// Структура для запису файлу в кореневій директорії
struct FAT16DirEntry {
    char DIR_Name[11];      // Ім'я файлу
    union {
        struct {
            uint8_t ATTR_READ_ONLY : 1;   // 0x01
            uint8_t ATTR_HIDDEN : 1;      // 0x02
            uint8_t ATTR_SYSTEM : 1;      // 0x04
            uint8_t ATTR_VOLUME_ID : 1;   // 0x08
            uint8_t ATTR_DIRECTORY : 1;    // 0x10
            uint8_t ATTR_ARCHIVE : 1;      // 0x20
            uint8_t ATTR_RESERVED : 2;     // Залишкові біти
        };
        uint8_t DIR_Attr; // Повний байт для атрибутів
    };
    uint8_t DIR_NTRes;
    uint8_t DIR_CrtTimeTenth;
    uint16_t DIR_CrtTime;
    uint16_t DIR_CrtDate;
    uint16_t DIR_LstAccDate;
    uint16_t DIR_FstClusHI; // Перший кластер
    uint16_t DIR_WrtTime;
    uint16_t DIR_WrtDate;
    uint16_t DIR_FstClusLO;
    uint32_t DIR_FileSize;  // Розмір файлу
};



struct FAT12DirEntry {
    char DIR_Name[11];      // Ім'я файлу
    union {
        struct {
            uint8_t ATTR_READ_ONLY : 1;   // 0x01
            uint8_t ATTR_HIDDEN : 1;      // 0x02
            uint8_t ATTR_SYSTEM : 1;      // 0x04
            uint8_t ATTR_VOLUME_ID : 1;   // 0x08
            uint8_t ATTR_DIRECTORY : 1;    // 0x10
            uint8_t ATTR_ARCHIVE : 1;      // 0x20
            uint8_t ATTR_RESERVED : 2;     // Залишкові біти
        };
        uint8_t DIR_Attr; // Повний байт для атрибутів
    };
    uint8_t DIR_NTRes;
    uint8_t DIR_CrtTimeTenth;
    uint16_t DIR_CrtTime;
    uint16_t DIR_CrtDate;
    uint16_t DIR_LstAccDate;
    uint16_t DIR_FstClusHI; // Перший кластер
    uint16_t DIR_WrtTime;
    uint16_t DIR_WrtDate;
    uint16_t DIR_FstClusLO;
    uint32_t DIR_FileSize;  // Розмір файлу
};



struct FAT32DirEntry {
    char DIR_Name[11];      // Ім'я файлу
    uint8_t DIR_Attr;       // Атрибути
    uint8_t DIR_NTRes;      // Зарезервовано для NT
    uint8_t DIR_CrtTimeTenth; // Десяті частки секунди створення
    uint16_t DIR_CrtTime;   // Час створення
    uint16_t DIR_CrtDate;   // Дата створення
    uint16_t DIR_LstAccDate; // Дата останнього доступу
    uint16_t DIR_FstClusHI; // Високий порядок першого кластера
    uint16_t DIR_WrtTime;   // Час запису
    uint16_t DIR_WrtDate;   // Дата запису
    uint16_t DIR_FstClusLO; // Низький порядок першого кластера
    uint32_t DIR_FileSize;  // Розмір файлу у байтах
};

// Структура LFN запису
struct LFNEntry {
    uint8_t sequence;          // Порядковий номер
    char name1[10];            // Перші 5 символів імені
    uint8_t attribute;         // Атрибут (0x0F для LFN)
    uint8_t type;              // Зарезервовано
    uint8_t checksum;          // Контрольна сума
    char name2[12];            // Наступні 6 символів імені
    uint16_t firstCluster;     // Зарезервовано
    char name3[4];             // Останні 2 символи імені
};

// Структура для зберігання необхідних даних про файл
struct FileEntry {
    std::string fileName;
    uint32_t firstCluster;
    uint32_t fileSize; // Вказаний розмір файлу в байтах
    std::vector<uint32_t> clusterChain; // Ланцюг кластерів
    std::string entryName;
    uint32_t entryCluster;
};


#pragma pack(pop)  // Повертає попереднє вирівнювання

#endif //FATCHECKER_FATSTRUCTS_HPP
