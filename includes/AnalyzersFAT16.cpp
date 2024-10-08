
#include <iostream>
#include "FATstructs.hpp"
#include <algorithm>

void AnalyzeMainFAT16(){
    std::cout<<"Analyzing Main FAT Table"<<std::endl;
};
void AnalyzeCopyFAT16(){
    std::cout<<"Analyzing Copy FAT Table"<<std::endl;
};
void AnalyzeRootDir16(){
    std::cout<<"Analyzing Root Dir"<<std::endl;
};
void AnalyzeDiskData16(){
    std::cout<<"Analyzing Disk Data"<<std::endl;
};

// Функція для перевірки, чи значення входить у допустимий список
template <typename T, size_t N>
bool isValid(T value, const T (&validArray)[N]) {
    return std::any_of(std::begin(validArray), std::end(validArray), [value](T v) { return v == value; });
}

bool isBootFAT16Invalid(extFAT12_16* bpb){
    // Функція перевіряє всі інваріанти бут сектора і повертає false якщо інформацію записано невірно і true якщо все добре
    bool isBootInvalid = false;

    uint16_t bytsPerSec = bpb->basic.BPB_BytsPerSec;
    uint8_t secPerClus = bpb->basic.BPB_SecPerClus;
    uint8_t numFATs = bpb->basic.BPB_NumFATs;
    uint16_t rootEntCnt = bpb->basic.BPB_RootEntCnt;
    uint16_t rsvdSecCnt = bpb->basic.BPB_RsvdSecCnt;
    uint16_t totSec16 = bpb->basic.BPB_TotSec16;
    uint32_t totSec32 = bpb->basic.BPB_TotSec32;
    uint8_t media = bpb->basic.BPB_Media;
    uint16_t fatSize = bpb->basic.BPB_FATSz16;
    uint16_t secPerTrk = bpb->basic.BPB_SecPerTrk;
    uint16_t numHeads = bpb->basic.BPB_NumHeads;
    uint8_t drvNum = bpb->BS_DrvNum;
    uint8_t reserved1 = bpb->BS_Reserved1;
    uint8_t bootSig = bpb->BS_BootSig;


    // Далі використовуємо ці змінні для перевірок
    const uint16_t validSectorSizes[] = {512, 1024, 2048, 4096};
    const uint8_t validSecPerClus[] = {1, 2, 4, 8, 16, 32, 64, 128};
    const uint8_t validMedia[] = {0xF0, 0xF8, 0xF9,
    0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF};


    // Перевірка BS_jmpBoot
    if (!(bpb->basic.BS_jmpBoot[0] == 0xEB && bpb->basic.BS_jmpBoot[2] == 0x90) && bpb->basic.BS_jmpBoot[0] != 0xE9){
        std::cerr << "Incorrect jump boot address"<<std::endl;
        isBootInvalid = true;
    }

    // Перевіряємо розмір сектора
    if (!isValid(bytsPerSec, validSectorSizes) ){
        std::cerr << "Incorrect size of sectors: " << bytsPerSec << std::endl;
        isBootInvalid = true;
    }

    // Перевіряємо кількість секторів на кластер
    if (!isValid(secPerClus, validSecPerClus)) {
        std::cerr << "Incorrect number of sectors per cluster: " << (int)secPerClus << std::endl;
        isBootInvalid = true;
    }

    // Перевіряємо кількість зарезервованих секторів
    if (rsvdSecCnt == 0) {
        std::cerr << "Incorrect number of reserved sectors: " << rsvdSecCnt << std::endl;
        isBootInvalid = true;
    }

    // Перевіряємо кількість FAT-таблиць
    if (numFATs == 0) {
        std::cerr << "Incorrect number of FAT tables: " << (int)numFATs << std::endl;
        isBootInvalid = true;
    }

    // Перевіряємо BPB_RootEntCnt
    uint32_t rootDirSize = rootEntCnt * 32;
    if (rootEntCnt == 0 || rootDirSize%bytsPerSec != 0 ){
        std::cerr << "Incorrect number of possible entries in the root directory: " <<(int)rootEntCnt<<std::endl;
        isBootInvalid = true;
    }

    // Перевіряємо BPB_Media
    if (!isValid(media, validMedia)){
        std::cerr << "Invalid media type: "<< std::hex<<media<<std::dec<<std::endl;
        isBootInvalid = true;
    }
    // Перевіряємо к-сть секторів для однієї FAT таблиці
    if (fatSize == 0) {
        std::cerr << "Invalid FAT size: " << fatSize << std::endl;
        isBootInvalid = true;
    }
    // Перевіряємо к-сть секторів на трек
    if (secPerTrk == 0) {
        std::cerr << "Invalid sectors per track: " << secPerTrk << std::endl;
        isBootInvalid = true;
    }

    // Перевіряємо к-сть головок
    if (numHeads == 0) {
        std::cerr << "Invalid number of heads: " << numHeads << std::endl;
        isBootInvalid = true;
    }

    // Перевіряємо загальну кількість секторів
    if (totSec32 < 65536 && totSec16 == 0){
        std::cerr << "Number of sectors can't be zero!" << std::endl;
        isBootInvalid = true;
    }

    // Перевіряємо BS_DrvNum
    if (drvNum != 0x80 && drvNum != 0x00) {
        std::cerr << "Invalid drive number: " << (int)drvNum << std::endl;
        isBootInvalid = true;
    }

    // Перевіряємо BS_Reserved1
    if (reserved1 != 0x00) {
        std::cerr << "Invalid reserved field: " << reserved1 << std::endl;
        isBootInvalid = true;
    }

    // Перевіряємо BS_BootSig =  0x29 якщо один з наступних = 0x00
    if (bootSig == 0x29 && bpb->BS_VolID == 0x00 && bpb->BS_VolLab[0] == 0x00) { // Assuming you have this in your struct
        std::cerr << "Invalid boot signature: " << (int)bootSig << std::endl;
        isBootInvalid = true;
    }

    // (Потім) Перевірка Signature_word

    // (Потім) Перевірка останнього біта - 0x00 якщо BPB_BytsPerSec > 512

    return isBootInvalid;
};