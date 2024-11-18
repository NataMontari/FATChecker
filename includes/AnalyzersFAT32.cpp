#include <iostream>
#include "FATstructs.hpp"
#include <algorithm>
#include <cstring>
#include <vector>
#include <string>
#include <set>
#include <unordered_map>
#include <unordered_set>

// аналіз FAT таблиць і порівняння копій
bool analyzeFAT32Tables(const std::vector<uint32_t*>& FATs, int FATSize, uint16_t bytesPerSec, bool fixErrors) {
    std::cout << "=== Analyzing FAT32 Tables ===\n";
    if (FATs.empty()) {
        std::cerr << "No FAT tables found.\n";
        return false;
    }

    bool fixed = false;
    size_t numFATs = FATs.size();

    for (size_t i = 1; i < numFATs; ++i) {
        // порівняння FAT таблиць
        if (std::memcmp(FATs[0], FATs[i], sizeof(uint32_t) * (FATSize * bytesPerSec / sizeof(uint32_t))) != 0) {
            std::cerr << "FAT table " << i + 1 << " differs from FAT table 1.\n";
            if (fixErrors) {
                std::memcpy(FATs[i], FATs[0], sizeof(uint32_t) * (FATSize * bytesPerSec / sizeof(uint32_t)));
                std::cout << "FAT table " << i + 1 << " fixed by copying from FAT table 1.\n";
                fixed = true;
            } else {
                return false;
            }
        }
    }

    if (!fixed) {
        std::cout << "All FAT tables are consistent.\n";
    }
    return true;
}

// завантаження FAT таблиці у вектор
std::vector<uint32_t> loadFAT32Table(const uint8_t* fatBuffer, int fatSizeBytes) {
    std::vector<uint32_t> FAT;
    FAT.reserve(fatSizeBytes / 4);

    for (int i = 0; i < fatSizeBytes; i += 4) {
        uint32_t entry = static_cast<uint32_t>(fatBuffer[i]) |
                         (static_cast<uint32_t>(fatBuffer[i + 1]) << 8) |
                         (static_cast<uint32_t>(fatBuffer[i + 2]) << 16) |
                         (static_cast<uint32_t>(fatBuffer[i + 3]) << 24);
        FAT.push_back(entry);
    }

    return FAT;
}

void AnalyzeMainFAT32(const uint8_t* fatBuffer, const std::vector<uint8_t*>& fatCopies, int fatSize, uint16_t bytesPerSec, bool fixErrors) {
    std::cout << "Analyzing Main FAT32 Table...\n";

    // Завантаження основної FAT таблиці
    std::vector<uint32_t> mainFAT = loadFAT32Table(fatBuffer, fatSize * bytesPerSec);

    // Завантаження копій FAT таблиць
    std::vector<uint32_t*> FATs;
    FATs.push_back(mainFAT.data());
    for (auto* copy : fatCopies) {
        std::vector<uint32_t> fatCopy = loadFAT32Table(copy, fatSize * bytesPerSec);
        FATs.push_back(fatCopy.data());
    }

    // Аналіз таблиць
    bool result = analyzeFAT32Tables(FATs, fatSize, bytesPerSec, fixErrors);
    if (result) {
        std::cout << "FAT32 table analysis complete: No inconsistencies found.\n";
    } else {
        std::cerr << "FAT32 table analysis complete: Errors detected.\n";
    }
}

void AnalyzeCopyFAT32(){
    std::cout<<"Analyzing Copy FAT Table"<<std::endl;
};
bool AnalyzeRootDir32(FILE *file, uint32_t rootCluster, uint16_t bytesPerSec, uint8_t secPerClus, const uint32_t *FAT, uint32_t FATSize, std::vector<FAT32DirEntry>& rootDirEntries, bool fixErrors) {
    std::cout << "=== Analyzing Root Directory in FAT32 ===\n";

    const int ENTRY_SIZE = sizeof(FAT32DirEntry);  // Розмір одного запису
    const uint32_t bytesPerCluster = bytesPerSec * secPerClus;  // Розмір одного кластера
    uint8_t buffer[bytesPerCluster];  // Буфер для зчитування кластерів
    uint32_t currentCluster = rootCluster;  // Поточний кластер
    bool isRootValid = true;
    std::set<std::string> fileNamesSet;  // Для перевірки дублювання імен

    while (currentCluster < 0xFFFFFF8) {  // Поки кластер не є кінцевим маркером
        // Позиціювання у файл для зчитування
        uint32_t startSector = currentCluster * secPerClus;
        uint64_t address = startSector * bytesPerSec;

        if (fseek(file, address, SEEK_SET) < 0) {
            std::cerr << "Failed to seek to root directory cluster: " << currentCluster << "\n";
            isRootValid = false;
            break;
        }

        // Зчитування кластеру
        if (fread(buffer, 1, bytesPerCluster, file) != bytesPerCluster) {
            std::cerr << "Failed to read root directory cluster: " << currentCluster << "\n";
            isRootValid = false;
            break;
        }

        // Обробка записів у кластері
        for (int i = 0; i < bytesPerCluster; i += ENTRY_SIZE) {
            FAT32DirEntry entry;
            std::memcpy(&entry, buffer + i, ENTRY_SIZE);

            if (entry.DIR_Name[0] == 0x00) {  // Порожній запис (кінець)
                break;
            }

            if (static_cast<unsigned char>(entry.DIR_Name[0]) == 0xE5) {  // Видалений запис
                continue;
            }

            if (entry.DIR_Attr == 0x0F) {  // Довге ім'я файлу (LFN)
                continue;  // Обробка довгих імен окремо
            }

            // Перевірка дублювання імен
            std::string fileName(entry.DIR_Name, 11);
            fileName.erase(std::remove(fileName.begin(), fileName.end(), ' '), fileName.end());
            if (fileNamesSet.find(fileName) != fileNamesSet.end()) {
                std::cerr << "Duplicate file name found: " << fileName << "\n";
                isRootValid = false;
                continue;
            }
            fileNamesSet.insert(fileName);

            // Додавання запису до вектора
            rootDirEntries.push_back(entry);

            // Перевірка атрибутів
            if ((entry.DIR_Attr & 0x3F) == 0) {  // Невалідні атрибути
                std::cerr << "Invalid attribute detected for entry: " << fileName << "\n";
                isRootValid = false;
            }
        }

        // Переходимо до наступного кластера
        currentCluster = FAT[currentCluster];
    }

    if (isRootValid) {
        std::cout << "Root directory is valid.\n";
    } else {
        std::cerr << "Errors detected in the root directory.\n";
    }

    return isRootValid;
}
bool readDataCluster32(FILE *file, uint16_t bytesPerSec, uint32_t startCluster, uint8_t secPerClus, uint32_t dataStartSector, std::vector<FAT32DirEntry> &entries) {
    const int ENTRY_SIZE = sizeof(FAT32DirEntry); // Розмір одного запису в байтах
    const int clusterSize = bytesPerSec * secPerClus; // Розмір одного кластера
    uint8_t buffer[clusterSize]; // Буфер для зчитування кластера

    uint64_t address = (dataStartSector + (startCluster - 2) * secPerClus) * bytesPerSec;

    // переходимо до потрібного кластера
    if (fseek(file, address, SEEK_SET) != 0) {
        std::cerr << "Failed to seek to cluster #" << startCluster << ".\n";
        return false;
    }

    // Зчитуємо кластер
    if (fread(buffer, 1, clusterSize, file) != clusterSize) {
        std::cerr << "Failed to read cluster #" << startCluster << ".\n";
        return false;
    }

    // Обробляємо записи в кластері
    for (int i = 0; i < clusterSize / ENTRY_SIZE; ++i) {
        FAT32DirEntry entry;
        std::memcpy(&entry, buffer + i * ENTRY_SIZE, ENTRY_SIZE);

        // Якщо це порожній запис, виходимо
        if (entry.DIR_Name[0] == 0x00) {
            break;
        }

        // Пропускаємо видалені записи
        if (static_cast<unsigned char>(entry.DIR_Name[0]) == 0xE5) {
            continue;
        }

        // Додаємо запис до списку
        entries.push_back(entry);
    }

    return true;
}

// Функція для аналізу кластера
void analyzeClusterUsage32(std::vector<uint32_t>& FAT, uint32_t FATSize, const std::vector<FAT32DirEntry>& directoryEntries, bool fixErrors) {
    std::cout << "=== Analyzing Cluster Usage ===\n";

    // Мапа для відстеження кластерів, що використовуються
    std::unordered_map<uint32_t, std::string> clusterToFileMap;
    std::unordered_set<uint32_t> usedClusters;
    std::unordered_set<uint32_t> freeClusters;

    // Аналіз використання кластерів
    for (const auto& entry : directoryEntries) {
        uint32_t currentCluster = (entry.DIR_FstClusHI << 16) | entry.DIR_FstClusLO;

        // Ігноруємо порожні або видалені записи
        if (currentCluster == 0 || static_cast<unsigned char>(entry.DIR_Name[0]) == 0xE5) {
            continue;
        }

        while (currentCluster < FATSize) {
            if (usedClusters.find(currentCluster) != usedClusters.end()) {
                std::cerr << "Error: Cluster " << currentCluster << " is used by multiple files.\n";
                if (fixErrors) {
                    std::cout << "Fixing cluster issue: Marking as free.\n";
                    FAT[currentCluster] = 0; // Позначення кластера як вільного
                }
            } else {
                usedClusters.insert(currentCluster);
                clusterToFileMap[currentCluster] = std::string(entry.DIR_Name, 11);
            }

            // Перехід до наступного кластера
            uint32_t nextCluster = FAT[currentCluster];
            if (nextCluster >= 0x0FFFFFF8) {
                break; // Кінець ланцюга
            }
            currentCluster = nextCluster;
        }
    }

    // Виявлення загублених кластерів
    for (uint32_t i = 2; i < FATSize; ++i) { // Кластери починаються з 2
        if (usedClusters.find(i) == usedClusters.end() && FAT[i] != 0) {
            std::cerr << "Warning: Cluster " << i << " is marked as used but not referenced by any file.\n";
            if (fixErrors) {
                std::cout << "Fixing: Marking cluster " << i << " as free.\n";
                FAT[i] = 0; // Позначення кластера як вільного
            }
        }
    }

    std::cout << "Cluster analysis complete.\n";
}
void checkLostClusters(const std::vector<uint32_t>& FAT, uint32_t FATSize, const std::unordered_set<uint32_t>& usedClusters, bool fixErrors) {
    std::cout << "=== Checking for Lost Clusters ===" << std::endl;

    // Список загублених кластерів
    std::vector<uint32_t> lostClusters;

    for (uint32_t i = 2; i < FATSize; ++i) { // Ігноруємо зарезервовані кластери (0 і 1)
        if (usedClusters.find(i) == usedClusters.end() && FAT[i] != 0) {
            lostClusters.push_back(i);
            std::cerr << "Warning: Lost cluster detected: " << i << std::endl;
            if (fixErrors) {
                std::cout << "Fixing: Marking cluster " << i << " as free." << std::endl;
                const_cast<uint32_t&>(FAT[i]) = 0; // Позначення кластера як вільного
            }
        }
    }

    if (lostClusters.empty()) {
        std::cout << "No lost clusters detected." << std::endl;
    } else {
        std::cerr << "Total lost clusters detected: " << lostClusters.size() << std::endl;
    }
}

void AnalyzeDiskData32(FILE *file, uint16_t bytesPerSec, uint8_t secPerClus, uint32_t dataStartSector, const std::vector<FAT32DirEntry> &dirEntries, uint32_t *FAT, uint32_t FATSize, bool fixErrors) {
    std::cout << "=== Analyzing Disk Data (Clusters) ===" << std::endl;

    std::vector<FAT32DirEntry> subDirEntries; // Для зберігання записів піддиректорій
    std::vector<uint32_t> visitedClusters;   // Для відстеження пройдених кластерів
    bool isDataValid = true;

    // Аналіз файлів і директорій
    for (const auto &entry : dirEntries) {
        if (entry.DIR_Name[0] == 0x00 || entry.DIR_Name[0] == 0xE5) {
            continue; // Пропускаємо порожні та видалені записи
        }

        uint32_t firstCluster = (entry.DIR_FstClusHI << 16) | entry.DIR_FstClusLO;
        if (firstCluster < 2 || firstCluster >= FATSize) {
            std::cerr << "Error: Invalid cluster number for entry " << entry.DIR_Name << std::endl;
            isDataValid = false;
            continue;
        }

        if (std::find(visitedClusters.begin(), visitedClusters.end(), firstCluster) != visitedClusters.end()) {
            std::cerr << "Warning: Cluster " << firstCluster << " already used by another file." << std::endl;
            if (fixErrors) {
                std::cerr << "Fixing error by marking the cluster as free." << std::endl;
                FAT[firstCluster] = 0;
            }
            isDataValid = false;
            continue;
        }
        visitedClusters.push_back(firstCluster);

        if (!readDataCluster32(file, bytesPerSec, firstCluster, secPerClus, dataStartSector, subDirEntries)) {
            std::cerr << "Failed to read cluster data for cluster #" << firstCluster << ".\n";
            isDataValid = false;
            continue;
        }

        if (entry.DIR_Attr & 0x10) { // Якщо це директорія
            if (!subDirEntries.empty()) {
                AnalyzeDiskData32(file, bytesPerSec, secPerClus, dataStartSector, subDirEntries, FAT, FATSize, fixErrors);
            }
        }

        subDirEntries.clear();
    }

    // Перевірка загублених кластерів
    std::unordered_set<uint32_t> usedClusters(visitedClusters.begin(), visitedClusters.end());
    checkLostClusters(std::vector<uint32_t>(FAT, FAT + FATSize), FATSize, usedClusters, fixErrors);

    if (isDataValid) {
        std::cout << "Disk data analysis complete: No errors found." << std::endl;
    } else {
        std::cerr << "Disk data analysis complete: Errors detected." << std::endl;
    }
}



// функція для перевірки, чи значення входить у допустимий список
template <typename T, size_t N>
bool isValid(T value, const T (&validArray)[N]) {
    return std::any_of(std::begin(validArray), std::end(validArray), [value](T v) { return v == value; });
}

bool isBootFAT32Invalid(extFAT32* bpb){
    // функція перевіряє всі інваріанти бут сектора і повертає false якщо інформацію записано невірно і true якщо все добре
    bool isBootInvalid = false;

    uint16_t bytsPerSec = bpb->basic.BPB_BytsPerSec;
    uint8_t secPerClus = bpb->basic.BPB_SecPerClus;
    uint8_t numFATs = bpb->basic.BPB_NumFATs;
    uint16_t rootEntCnt = bpb->basic.BPB_RootEntCnt;
    uint16_t rsvdSecCnt = bpb->basic.BPB_RsvdSecCnt;
    uint16_t totSec16 = bpb->basic.BPB_TotSec16;
    uint32_t totSec32 = bpb->basic.BPB_TotSec32;
    uint8_t media = bpb->basic.BPB_Media;
    uint16_t fatSize16 = bpb->basic.BPB_FATSz16;
    uint16_t fatSize32 = bpb->BPB_FATSz32;
    uint16_t secPerTrk = bpb->basic.BPB_SecPerTrk;
    uint16_t numHeads = bpb->basic.BPB_NumHeads;
    uint8_t drvNum = bpb->BS_DrvNum;
    uint8_t reserved1 = bpb->BS_Reserved1;
    uint8_t bootSig = bpb->BS_BootSig;
    uint32_t rootClusNum = bpb->BPB_RootClus;


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

    // Перевіряємо к-сть секторів на кластер
    if (!isValid(secPerClus, validSecPerClus)) {
        std::cerr << "Incorrect number of sectors per cluster: " << (int)secPerClus << std::endl;
        isBootInvalid = true;
    }

    // Перевіряємо к-сть зарезервованих секторів
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
    if (rootEntCnt != 0){
        std::cerr << "Invalid BPB_RootEntCnt, should be 0 on FAT32: " <<(int)rootEntCnt<<std::endl;
        isBootInvalid = true;
    }

    // Перевіряємо BPB_Media
    if (!isValid(media, validMedia)){
        std::cerr << "Invalid media type: "<< std::hex<<media<<std::dec<<std::endl;
        isBootInvalid = true;
    }
    // Перевіряємо к-сть секторів для однієї FAT таблиці
    if (fatSize16 != 0) {
        std::cerr << "Invalid BPB_FATSz16, should be 0 on FAT32: " << (int)fatSize16 << std::endl;
        isBootInvalid = true;
    }
    if (fatSize32 == 0 ){
        std::cerr << "Invalid FAT size: " << (int)fatSize32 << std::endl;
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

    // Перевіряємо загальну к-сть секторів
    if (totSec16 != 0){
        std::cerr << "Invalid BPB_TotSec16, value here should be 0!" << std::endl;
        isBootInvalid = true;
    }
    if (totSec32 == 0){
        std::cerr << "Number of sectors can't be zero!" << std::endl;
        isBootInvalid = true;
    }

    // Перевіряємо к-сть секторів для всіх FAT таблиць
    if (numFATs * fatSize32 >= totSec32 ){
        std::cerr << "Number of sectors for FAT tables is equal or greater than number of all sectors"<<std::endl;
        isBootInvalid = true;
    }

    if (rootClusNum <= 1){
        std::cerr << " Invalid number of root cluster: "<< (int)rootClusNum<<std::endl;
        isBootInvalid = true;
    }

    // Перевіряємо BS_DrvNum
    if (drvNum != 0x80 && drvNum != 0x00) {
        std::cerr << "Invalid drive number: " << (int)drvNum << std::endl;
        isBootInvalid = true;
    }

    // Перевіряємо BS_Reserved1
    if (reserved1 != 0x00) {
        std::cerr << "Invalid reserved field: " << static_cast<int>(reserved1) << std::endl;
        if (static_cast<int>(reserved1) != 0x00) {
            std::cout << "Fixing reserved field to 0." << std::endl;
            bpb->BS_Reserved1 = 0x00; // Виправлення (не паше поки)
            isBootInvalid = false;
        }
        // isBootInvalid = false;
        // isBootInvalid = true;// правильна валідація
    }

    // Перевіряємо BS_BootSig =  0x29 якщо один з наступних = 0x00
    if (bootSig == 0x29 && bpb->BS_VolID == 0x00 && bpb->BS_VolLab[0] == 0x00) {
        std::cerr << "Invalid boot signature: " << (int)bootSig << std::endl;
        isBootInvalid = true;
    }

    // (Потім) Перевірка Signature_word

    // (Потім) Перевірка останнього біта - 0x00 якщо BPB_BytsPerSec > 512

    return isBootInvalid;
}
bool readFAT32Tables(FILE *file, std::vector<uint32_t*>& FATs, int FATSize, int startSector, int numberOfFATs, uint16_t bytesPerSec) {
    std::cout << "Reading FAT32 Tables..." << std::endl;

    const int startFATPosition = startSector * bytesPerSec;
    for (int i = 0; i < numberOfFATs; i++) {
        uint32_t *FAT = new uint32_t[(FATSize * bytesPerSec) / sizeof(uint32_t)];

        if (fseek(file, startFATPosition + FATSize * i * bytesPerSec, SEEK_SET) == -1 ||
            fread(FAT, sizeof(uint32_t), FATSize * bytesPerSec / sizeof(uint32_t), file) != FATSize * bytesPerSec / sizeof(uint32_t)) {
            perror("Failed to read FAT32 table");
            delete[] FAT;
            return false;
        }
        FATs.push_back(FAT);
    }

    std::cout << "Successfully read FAT32 tables." << std::endl;
    return true;
}