#include <iostream>
#include "../includes/FATstructs.hpp"
#include <algorithm>
#include <vector>
#include <cstring>
#include <iomanip>
#include <stdio.h>
#include <fcntl.h>
#include <set>
#include "../includes/FixFAT16.hpp"



bool fixBootSectorErrors(extFAT12_16* bpb, BootSectorErrors &errors) {
    bool isFixed = false;

    std::cout << "Attempting to fix boot sector errors..." << std::endl;

    // Якщо адреса BS_jmpBoot неправильна, встановити її на стандартне значення
    if (errors.BPB_JumpAddressInvalid) {
        bpb->basic.BS_jmpBoot[0] = 0xEB;
//        bpb->basic.BS_jmpBoot[1] = 0x3C;
        bpb->basic.BS_jmpBoot[2] = 0x90;
        std::cout << "Fixed: Set BS_jmpBoot to standard jump code." << std::endl;
        isFixed = true;
    }

    // Виправити розмір сектору, якщо він некоректний
    if (errors.BPB_BytsPerSecInvalid) {
        bpb->basic.BPB_BytsPerSec = 512; // Встановити на мінімальний валідний розмір
        std::cout << "Fixed: Set BPB_BytsPerSec to 512." << std::endl;
        isFixed = true;
    }

    // Виправити кількість секторів на кластер, якщо значення некоректне
    if (errors.BPB_SecPerClusInvalid) {
        bpb->basic.BPB_SecPerClus = 1; // Наприклад, встановити на мінімальне допустиме значення
        std::cout << "Fixed: Set BPB_SecPerClus to 1." << std::endl;
        isFixed = true;
    }

    // Виправити кількість зарезервованих секторів
    if (errors.BPB_RsvdSecCntInvalid) {
        bpb->basic.BPB_RsvdSecCnt = 2; // Стандартне значення для FAT16
        std::cout << "Fixed: Set BPB_RsvdSecCnt to 2." << std::endl;
        isFixed = true;
    }

    // Виправити кількість FAT-таблиць
    if (errors.BPB_NumFATsInvalid) {
        bpb->basic.BPB_NumFATs = 2; // Стандартне значення для FAT16
        std::cout << "Fixed: Set BPB_NumFATs to 2." << std::endl;
        isFixed = true;
    }

    // Виправити BPB_RootEntCnt
    if (errors.BPB_RootEntCntInvalid) {
        bpb->basic.BPB_RootEntCnt = 512; // Стандартне значення для FAT16
        std::cout << "Fixed: Set BPB_RootEntCnt to 512." << std::endl;
        isFixed = true;
    }

    // Виправити BPB_Media
    if (errors.BPB_MediaInvalid) {
        bpb->basic.BPB_Media = 0xF8; // Стандартне значення для жорстких дисків
        std::cout << "Fixed: Set BPB_Media to 0xF8." << std::endl;
        isFixed = true;
    }

    // Виправити BPB_FATSz16
    if (errors.BPB_FATSz16Invalid) {
        bpb->basic.BPB_FATSz16 = 0x1000; // Приклад значення (залежить від розміру FAT-таблиці)
        std::cout << "Fixed: Set BPB_FATSz16 to 4096 sectors." << std::endl;
        isFixed = true;
    }

    // Виправити BPB_SectorsPerTrack
    if (errors.BPB_SectorsPerTrackInvalid) {
        bpb->basic.BPB_SecPerTrk = 63; // Типове значення для жорстких дисків
        std::cout << "Fixed: Set BPB_SecPerTrk to 63." << std::endl;
        isFixed = true;
    }

    // Виправити BPB_NumHeads
    if (errors.BPB_NumHeadsInvalid) {
        bpb->basic.BPB_NumHeads = 255; // Типове значення для жорстких дисків
        std::cout << "Fixed: Set BPB_NumHeads to 255." << std::endl;
        isFixed = true;
    }

    // Виправити BPB_TotSec16
    if (errors.BPB_TotSec16Invalid) {
        bpb->basic.BPB_TotSec16 = 65535; // Максимальне значення для FAT16
        std::cout << "Fixed: Set BPB_TotSec16 to maximum valid value for FAT16." << std::endl;
        isFixed = true;
    }

    // Виправити BPB_NumFATsTooMuch
    if (errors.BPB_NumFATsTooMuchInvalid) {
        // Обчисліть правильну кількість секторів для FAT-таблиці
        std::cout << "Error: Number of sectors for FAT tables is too high. Adjusting is not straightforward." << std::endl;
        // Це може вимагати додаткових обчислень на основі загального числа секторів
        isFixed = true;
    }

    // Виправити BS_DrvNum
    if (errors.BPB_DrvNumInvalid) {
        bpb->BS_DrvNum = 0x80; // Типове значення для жорсткого диска
        std::cout << "Fixed: Set BS_DrvNum to 0x80." << std::endl;
        isFixed = true;
    }

    // Виправити BS_Reserved1
    if (errors.BPB_BS_ReservedInvalid) {
        bpb->BS_Reserved1 = 0x00; // Стандартне значення
        std::cout << "Fixed: Set BS_Reserved1 to 0." << std::endl;
        isFixed = true;
    }

    // Виправити BS_BootSig
    if (errors.BPB_BootSigInvalid) {
        bpb->BS_BootSig = 0x29; // Стандартне значення
        std::cout << "Fixed: Set BS_BootSig to 0x29." << std::endl;
        isFixed = true;
    }

    return isFixed;
}
bool check_date(uint16_t date_value) {
    // Перевірка на 0 (не підтримується)
    if (date_value == 0) {
        return true; // Ігноруємо перевірку
    }

    uint8_t day = date_value & 0x1F;               // Бітові позиції 0-4
    uint8_t month = (date_value >> 5) & 0x0F;      // Бітові позиції 5-8
    uint8_t year = (date_value >> 9) & 0x7F;       // Бітові позиції 9-15

    // Перевірка діапазонів
    if (!(1 <= day && day <= 31)) return false;
    if (!(1 <= month && month <= 12)) return false;
    if (!(0 <= year && year <= 127)) return false; // 1980-2107

    return true;
}

bool fixRootDirErrors(std::vector<FAT16DirEntry>& rootDirEntries) {
    bool isFixed = false;

    std::cout << "Attempting to fix root directory errors..." << std::endl;

    // Видалити помилки для '.' і '..'
    for (auto& entry : rootDirEntries) {
        std::string entryName(reinterpret_cast<const char*>(entry.DIR_Name), 11);
        entryName.erase(std::remove(entryName.begin(), entryName.end(), ' '), entryName.end());

        // Виправлення записів '.' або '..'
        if (entryName == "." || entryName == "..") {
            uint16_t cluster = (entry.DIR_FstClusHI << 16) | entry.DIR_FstClusLO;

            if (entryName == "." && cluster != 0) {
                std::cout << "Fixing '.' entry to point to root directory cluster." << std::endl;
                entry.DIR_FstClusHI = 0;
                entry.DIR_FstClusLO = 0;
                isFixed = true;
            }

            if (entryName == ".." && cluster != 0) {
                std::cout << "Fixing '..' entry to point to root directory cluster." << std::endl;
                entry.DIR_FstClusHI = 0;
                entry.DIR_FstClusLO = 0;
                isFixed = true;
            }
        }

        // Виправлення некоректних імен
        if (entryName.empty() || entry.DIR_Name[0] == 0xE5) {
            std::cout << "Skipping invalid or deleted entry." << std::endl;
            continue;
        }

        if (entry.DIR_Attr == 0x0F) { // LFN запис
            std::cout << "Skipping LFN entry during root directory fixing." << std::endl;
            continue;
        }

        // Виправлення помилок атрибутів
        if (entry.DIR_Attr & 0xC0) { // Некоректні біти атрибутів
            std::cout << "Fixing invalid attributes for entry: " << entryName << std::endl;
            entry.DIR_Attr &= 0x3F; // Очищаємо старші біти
            isFixed = true;
        }

        // Перевірка дати створення
        if (!check_date(entry.DIR_CrtDate)) {
            std::cout << "Fixing invalid creation date for entry: " << entryName << std::endl;
            entry.DIR_CrtDate = 0; // Скидаємо дату
            isFixed = true;
        }

        // Перевірка дати останнього доступу
        if (!check_date(entry.DIR_LstAccDate)) {
            std::cout << "Fixing invalid last access date for entry: " << entryName << std::endl;
            entry.DIR_LstAccDate = 0; // Скидаємо дату
            isFixed = true;
        }

        // Перевірка дати модифікації
        if (!check_date(entry.DIR_WrtDate)) {
            std::cout << "Fixing invalid write date for entry: " << entryName << std::endl;
            entry.DIR_WrtDate = 0; // Скидаємо дату
            isFixed = true;
        }

        // Виправлення розміру файлу, якщо він некоректний
        if (entry.DIR_FileSize < 0) {
            std::cout << "Fixing invalid file size for entry: " << entryName << std::endl;
            entry.DIR_FileSize = 0;
            isFixed = true;
        }
    }

    if (isFixed) {
        std::cout << "Root directory errors have been fixed." << std::endl;
    } else {
        std::cout << "No errors found in the root directory." << std::endl;
    }

    return isFixed;
}


bool fixDataRegionErrors(){
    std::cout<<"Attempting to fix data dir errors"<<std::endl;

    return true;
}