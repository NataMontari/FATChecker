#include <iostream>
#include "FATstructs.hpp"
#include <algorithm>
#include <vector>
#include <cstring>
#include <iomanip>
#include <stdio.h>
#include <fcntl.h>
#include <set>
#include "FixFAT16.hpp"



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

bool fixRootDirErrors(){
    std::cout<<"Attempting to fix root dir errors"<<std::endl;

    return true;
}

bool fixDataRegionErrors(){
    std::cout<<"Attempting to fix data dir errors"<<std::endl;

    return true;
}