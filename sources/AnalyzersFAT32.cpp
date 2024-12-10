#include <iostream>
#include "../includes/FATstructs.hpp"
#include <algorithm>
#include <cstring>
#include <vector>
#include <string>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <iomanip>
#include "FixFAT32.hpp"
#include "AnalyzersFAT32.hpp"
#include "readFunctions.hpp"

// аналіз FAT таблиць і порівняння копій
bool analyzeFAT32Tables(const std::vector<uint32_t*>& FATs, int FATSize, uint16_t bytesPerSec, int startFATSector, bool fixErrors) {
    std::cout<<"---------------------------------------------------------"<<std::endl;
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
                writeFATTableToFile32(FATs[0], i, bytesPerSec, FATSize, startFATSector);
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
void printFileInfo32( FileEntry fileEntry, FAT32DirEntry entry)
{
    std::cout<<"---------------------------------------------------------"<<std::endl;
        std::cout << std::left << std::setw(30) << "File name: "
          << std::right << std::setw(20) << fileEntry.fileName << std::endl;
        std::string fileType;
        if (entry.ATTR_READ_ONLY) fileType += "Read-Only ";
        if (entry.ATTR_HIDDEN) fileType += "Hidden ";
        if (entry.ATTR_SYSTEM) fileType += "System ";
        if (entry.ATTR_VOLUME_ID) fileType += "Volume ID ";
        if (entry.ATTR_DIRECTORY) fileType += "Directory ";
        if (entry.ATTR_ARCHIVE) fileType += "File ";
        if (fileType.empty()) fileType = "Unknown";

        std::cout << std::left << std::setw(30) << "File Type: "
          << std::right << std::setw(20) << fileType << std::endl;

        std::cout << std::left << std::setw(30) << "File size: "
          << std::right << std::setw(20) << fileEntry.fileSize << std::endl;

        std::cout << std::left << std::setw(30) << "First Cluster: "
          << std::right << std::setw(20) << fileEntry.firstCluster << std::endl;

        uint16_t date = entry.DIR_CrtDate;
        int year = ((date >> 9) & 0x7F) + 1980;  // 15–9 bits
        int month = (date >> 5) & 0x0F;         // 8–5 bits

        int day = date & 0x1F;                  // 4–0 bits
        uint16_t time = entry.DIR_CrtTime;

        int hour = (time >> 11) & 0x1F;         // 15–11 bits
        int minute = (time >> 5) & 0x3F;        // 10–5 bits
        int second = (time & 0x1F) * 2;         // 4–0 bits (multiply by 2)

        std::ostringstream dateStream, timeStream;

        dateStream << std::setfill('0') << std::setw(2) << day << "-"
           << std::setw(2) << month << "-"
           << year;

        timeStream << std::setfill('0') << std::setw(2) << hour << ":"
           << std::setw(2) << minute << ":"
           << std::setw(2) << second;


        std::cout << std::left << std::setw(30) << "Creation Date: "
          << std::right << std::setw(20) << dateStream.str() << std::endl;


        std::cout << std::left << std::setw(30) << "Creation Time: "
          << std::right << std::setw(20) << timeStream.str() << std::endl;
};



void AnalyzeCopyFAT32(){
    std::cout<<"Analyzing Copy FAT Table"<<std::endl;
};

bool readRootDirectoryFAT32(FILE *file, uint16_t bytesPerSec, uint8_t secPerClus, uint32_t dataStartSector,
                            uint32_t rootCluster, const uint32_t *FAT, uint32_t FATSize, std::vector<FAT32DirEntry> &entries) {
    const int ENTRY_SIZE = sizeof(FAT32DirEntry);   // Розмір одного запису
    const int clusterSize = bytesPerSec * secPerClus; // Розмір одного кластера в байтах
    uint8_t buffer[clusterSize]; // Буфер для зчитування кластера

    uint32_t currentCluster = rootCluster; // Початковий кластер каталогу
    bool hasErrors = false;

    while (currentCluster < FATSize) { // Перевіряємо, щоб кластер був у межах FAT
        uint64_t address = (dataStartSector + (currentCluster - 2) * secPerClus) * bytesPerSec;

        // Перехід до потрібного кластера
        if (fseek(file, address, SEEK_SET) != 0) {
            std::cerr << "Failed to seek to cluster #" << currentCluster << ".\n";
            hasErrors = true;
            break;
        }

        // Зчитуємо дані кластера
        if (fread(buffer, 1, clusterSize, file) != clusterSize) {
            std::cerr << "Failed to read cluster #" << currentCluster << ".\n";
            hasErrors = true;
            break;
        }

        // Обробляємо записи в кластері
        for (int i = 0; i < clusterSize / ENTRY_SIZE; ++i) {
            FAT32DirEntry entry;
            std::memcpy(&entry, buffer + i * ENTRY_SIZE, ENTRY_SIZE);

            // Якщо це порожній запис, завершити зчитування
            if (entry.DIR_Name[0] == 0x00) {
                return true;
            }

            // Пропускаємо видалені записи
            if (static_cast<unsigned char>(entry.DIR_Name[0]) == 0xE5) {
                continue;
            }

            // Додаємо запис до списку
            entries.push_back(entry);
        }

        // Переходимо до наступного кластера за FAT-таблицею
        currentCluster = FAT[currentCluster];
        if (currentCluster >= 0x0FFFFFF8) { // Кінець ланцюжка кластерів FAT32
            break;
        }
    }

    return !hasErrors;
}


bool readDataCluster32(FILE *file, uint16_t bytesPerSec, uint32_t startCluster, uint8_t secPerClus, uint32_t dataStartSector, std::vector<FAT32DirEntry> &entries) {
    const int ENTRY_SIZE = sizeof(FAT32DirEntry); // розмір одного запису в байтах
    const int clusterSize = bytesPerSec * secPerClus; // розмір одного кластера
    uint8_t buffer[clusterSize]; // буфер для зчитування кластера

    uint64_t address = (dataStartSector + (startCluster - 2) * secPerClus) * bytesPerSec;

    // переходимо до потрібного кластера
    if (fseek(file, address, SEEK_SET) != 0) {
        std::cerr << "Failed to seek to cluster #" << startCluster << ".\n";
        return false;
    }

    if (fread(buffer, 1, clusterSize, file) != clusterSize) {
        std::cerr << "Failed to read cluster #" << startCluster << ".\n";
        return false;
    }

    // обробляємо записи в кластері
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
bool check_date32(uint16_t date_value) {
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

bool check_time32(uint16_t time_value) {
    // Перевірка на 0 (не підтримується)
    if (time_value == 0) {
        return true; // Ігноруємо перевірку
    }

    uint8_t seconds = time_value & 0x1F;            // Бітові позиції 0-4
    uint8_t minutes = (time_value >> 5) & 0x3F;     // Бітові позиції 5-10
    uint8_t hours = (time_value >> 11) & 0x1F;      // Бітові позиції 11-15

    // Перевірка діапазонів
    if (!(0 <= seconds && seconds <= 29)) return false; // 0-29 (0-58 секунд)
    if (!(0 <= minutes && minutes <= 59)) return false;
    if (!(0 <= hours && hours <= 23)) return false;

    return true;
}

bool checkEntry32(const FAT32DirEntry& entry){
    bool isEntryValid = true;
    // Check reserved attribute

    // Validate creation time and date
    if (entry.DIR_CrtTimeTenth < 0 || entry.DIR_CrtTimeTenth > 199) {
        std::cout << "Error: DIR_CrtTimeTenth out of range." << std::endl;
        isEntryValid = false;
    }

    // Validate the first cluster high word (must be 0 for FAT16)
    if (entry.DIR_FstClusHI != 0) {
        std::cout << "Error: DIR_FstClusHI must be 0 on FAT16." << std::endl;
        isEntryValid = false;
    }
    // Validate creation date
    if (!check_date32(entry.DIR_CrtDate)) {
        std::cout << "Error: Invalid creation date." << std::endl;
        isEntryValid = false;
    }

    // Validate last access date
    if (!check_date32(entry.DIR_LstAccDate)) {
        std::cout << "Error: Invalid last access date." << std::endl;
        isEntryValid = false;
    }

    // Validate write date
    if (!check_date32(entry.DIR_WrtDate)) {
        std::cout << "Error: Invalid write date." << std::endl;
        isEntryValid = false;
    }

    // Validate creation time
    if (!check_time32(entry.DIR_CrtTime)) {
        std::cout << "Error: Invalid creation time." << std::endl;
        isEntryValid = false;
    }

    // Validate write time
    if (!check_time32(entry.DIR_WrtTime)) {
        std::cout << "Error: Invalid write time." << std::endl;
        isEntryValid = false;
    }


    return isEntryValid;
}

bool isClusterValid32(uint32_t cluster, uint32_t maxCluster) {
    // Перевірка валідності кластера
    return cluster >= 2 && cluster < 0xFFFFFF8 && cluster <= maxCluster;
}
bool isValidName32(const std::string& name, int longNameFlag) {
    // FAT16 ім'я має бути рівно 11 символів (8 для імені і 3 для розширення)
    const char* validChars = "$%'-_@~`!(){}^#&.";  // Дозволені спеціальні символи


    // FAT16 ім'я не повинно починатися з пробілу або містити недопустимі символи
    if (name[0] == 0x20) {
        std::cout << "Error: Name starts with a space." << std::endl;
        return false;
    }
    std::string fileName = name;
    fileName.erase(std::remove(fileName.begin(), fileName.end(), ' '), fileName.end()); // Видаляємо пробіли

    if (longNameFlag){
        for (auto &c: fileName){
            // Допускаємо великі літери, цифри або спеціальні символи
            if (!std::isalpha(c) && !std::isdigit(c) && strchr(validChars, c) == nullptr) {
                return false;
            }
        }
    }
    else{
        for (auto &c: fileName){
            // Допускаємо великі літери, цифри або спеціальні символи
            if (!std::isupper(c) && !std::isdigit(c) && strchr(validChars, c) == nullptr) {
                return false;
            }
        }
    }
    // Якщо всі символи пройшли перевірку, ім'я файлу є валідним
    return true;
}
// Функція для злиття частин LFN
std::string extractLFN32(const LFNEntry& entry) {
    std::string name;

    // Додаємо перші 5 символів (name1)
    for (int i = 0; i < 5; ++i) {
        if (static_cast<unsigned char>(entry.name1[2 * i]) == 0xFF && static_cast<unsigned char>(entry.name1[2 * i + 1]) == 0xFF) break; // Кінець
        name += entry.name1[2 * i];  // UTF-16, але ми використовуємо перший байт
    }

    // Додаємо наступні 6 символів (name2)
    for (int i = 0; i < 6; ++i) {
        if (static_cast<unsigned char>(entry.name2[2 * i]) == 0xFF && static_cast<unsigned char>(entry.name2[2 * i + 1]) == 0xFF) break; // Кінець
        name += entry.name2[2 * i];
    }

    // Додаємо останні 2 символи (name3)
    for (int i = 0; i < 2; ++i) {
        if (static_cast<unsigned char>(entry.name3[2 * i]) == 0xFF && static_cast<unsigned char>(entry.name3[2 * i + 1]) == 0xFF) break; // Кінець
        name += entry.name3[2 * i];
    }

    return name;
}
bool AnalyzeRootDir32(FILE *file, uint32_t rootCluster, uint32_t dataStartSector, uint16_t bytesPerSec, uint8_t secPerClus, const uint32_t *FAT, uint32_t FATSize, std::vector<FAT32DirEntry>& rootDirEntries, std::vector<FAT32DirEntry>& dataDirEntries, std::vector<FileEntry>& fileEntries, bool fixErrors) {
    std::cout << "=== Analyzing Root Directory (FAT32) ===" << std::endl;

    // // Зчитуємо записи кореневого каталогу
    // if (!readDataCluster32(file, bytesPerSec, rootCluster, secPerClus, dataStartSector, rootDirEntries)) {
    //     std::cerr << "Failed to read root directory cluster." << std::endl;
    //     return false;
    // }
    if(!readRootDirectoryFAT32(file, bytesPerSec, secPerClus, dataStartSector, rootCluster, FAT, FATSize * bytesPerSec / sizeof(uint32_t), rootDirEntries))
    {
        std::cerr << "Failed to read root directory cluster." << std::endl;
        return false;
    }

    std::unordered_set<std::string> fileNamesSet; // унікальності імен
    std::vector<std::string> lfnParts;  // Для зберігання частин довгого імені
    std::string longFileName;
    std::vector<std::string> longFileNames;
    FileEntry fileEntry;
    int volumeIDCount = 0;
    int directoryCount = 0;
    int fileCount = 0;

    bool isRootDirValid = true;

    for (auto &entry : rootDirEntries) {
        if (entry.DIR_Name[0] == 0x00) {
            break; // порожній запис
        }

        if (static_cast<unsigned char>(entry.DIR_Name[0]) == 0xE5) {
            continue; // видалений запис
        }

        std::string entryName(reinterpret_cast<const char *>(entry.DIR_Name), 11);
        entryName = entryName.substr(0, entryName.find(' '));

        if (entryName == ".") {
            std::cout << "Error: Root directory contains a dot (.) entry." << std::endl;
            isRootDirValid = false;

            if (fixErrors) {
                uint16_t cluster = (entry.DIR_FstClusHI << 16) | entry.DIR_FstClusLO;
                if (isClusterValid32(cluster, 0xFFFFFF5)) {
                    std::cout << "The entry named '.' points to a valid cluster. Would you like to rename it? (y/n): ";
                    char response;
                    std::cin >> response;

                    if (response == 'y' || response == 'Y') {
                        std::cout << "Enter new name (max 11 characters): ";
                        std::string newName;
                        std::cin >> newName;

                        if (newName.length() > 11) {
                            newName = newName.substr(0, 11); // Обрізаємо до 11 символів
                            std::cout << "Name truncated to 11 characters: " << newName << std::endl;
                        }

                        memset(entry.DIR_Name, ' ', 11); // Заповнюємо пробілами
                        memcpy(entry.DIR_Name, newName.c_str(), newName.length());
                        std::cout << "Fixed: Entry renamed to: " << newName << std::endl;
                    }
                } else {
                    entry.DIR_Name[0] = 0xe5; // Позначаємо запис видаленим
                    std::cout << "Fixed: Entry points to an invalid cluster and was marked as deleted." << std::endl;

                }
            }
        }
        if (entryName == "..") {
            std::cout << "Error: Root directory contains a dotdot (..) entry." << std::endl;
            isRootDirValid = false;

            if (fixErrors) {
                uint16_t cluster = (entry.DIR_FstClusHI << 16) | entry.DIR_FstClusLO;
                if (isClusterValid32(cluster, 0xFFFFFF5)) {
                    std::cout << "The entry named '..' points to a valid cluster. Would you like to rename it? (y/n): ";
                    char response;
                    std::cin >> response;

                    if (response == 'y' || response == 'Y') {
                        std::cout << "Enter new name (max 11 characters): ";
                        std::string newName;
                        std::cin >> newName;

                        if (newName.length() > 11) {
                            newName = newName.substr(0, 11); // Обрізаємо до 11 символів
                            std::cout << "Name truncated to 11 characters: " << newName << std::endl;
                        }

                        memset(entry.DIR_Name, ' ', 11); // Заповнюємо пробілами
                        memcpy(entry.DIR_Name, newName.c_str(), newName.length());
                        std::cout << "Fixed: Entry renamed to: " << newName << std::endl;
                    }
                } else {
                    entry.DIR_Name[0] = 0xe5; // Позначаємо запис видаленим
                    std::cout << "Fixed: Entry points to an invalid cluster and was marked as deleted." << std::endl;
                }
            }
        }
        // Якщо це LFN запис (атрибут = 0x0F), збираємо довге ім'я
        if (entry.DIR_Attr == 0x0F) {
            const auto* lfn = reinterpret_cast<const LFNEntry*>(&entry);
            lfnParts.push_back(extractLFN32(*lfn));
            continue;
        }

        // Якщо це основний запис файлу або директорії, обробляємо його
        if (!lfnParts.empty()) {
            // Склеїмо всі частини імені у правильному порядку
            for (auto it = lfnParts.rbegin(); it != lfnParts.rend(); ++it) {
                longFileName += *it;
            }
            lfnParts.clear();  // Очищаємо частини імені після використання

            // Перевірка імені
            if (!isValidName32(longFileName, 1)) {
                std::cout << "Error: Invalid long file name: " << longFileName << std::endl;
                isRootDirValid = false;
            }

            // Перевірка на дублікати імен файлів
            if (fileNamesSet.find(longFileName) != fileNamesSet.end()) {
                std::cout << "Error: Duplicate name found: " << longFileName << std::endl;
                isRootDirValid = false;
            } else {
                fileNamesSet.insert(longFileName);
            }

            longFileNames.push_back(longFileName);
            // Виводимо довге ім'я файлу або директорії
#ifdef DEBUG_PRNT
            std::cout << "Long file name: " << longFileName << std::endl;
#endif

        }

        // Перевірка короткого імені
        std::string fileName(entry.DIR_Name, 11);

        if (!isValidName32(fileName, 0)) {
            std::cout << "Error: Invalid short file name: " << fileName << std::endl;

        }

        // Перевірка на коректність атрибутів
        if ((entry.DIR_Attr & 0x3F) == 0) { // Атрибути не повинні мати недопустимі значення
            std::cout << "Error: Invalid attribute detected for entry." << std::endl;
            isRootDirValid = false;

        }

        if(!checkEntry32(entry)){
            isRootDirValid = false;
        }

        if (entry.ATTR_DIRECTORY == 1) {
            directoryCount++;
            dataDirEntries.push_back(entry);
            // Перевірка на дублікати коротких імен файлів
            if (fileNamesSet.find(fileName) != fileNamesSet.end()) {
                std::cout << "Error: Duplicate name found: " << fileName << std::endl;
                isRootDirValid = false;
            } else {
                fileNamesSet.insert(fileName);
            }

            #ifdef DEBUG_PRNT
            std::cout <<"Dir:"<< std::hex << std::setw(2) << std::setfill('0') << (0xFF & static_cast<unsigned char>(entry.DIR_Name[0])) << " ";
            std::cout << std::dec << std::endl;
            std::cout<<entry.DIR_Name<<std::endl;
            #endif


        } else if(entry.ATTR_VOLUME_ID == 1){
            volumeIDCount++;
            if (entry.DIR_CrtDate != 0){
                std::cout<<"Warning! An error found: root entry shouldn't have any associated time stamps"<<std::endl;
                // isRootDirValid = false;
            }

            #ifdef DEBUG_PRNT
            std::cout <<"Volume:"<< std::hex << std::setw(2) << std::setfill('0') << (0xFF & static_cast<unsigned char>(entry.DIR_Name[0])) << " ";
            std::cout << std::dec << std::endl;
            std::cout<<entry.DIR_Name<<std::endl;
            #endif
        }
        else{

            // Перевірка на дублікати коротких імен файлів
            if (fileNamesSet.find(fileName) != fileNamesSet.end()) {
                std::cout << "Error: Duplicate name found: " << fileName << std::endl;
                isRootDirValid = false;
            } else {
                fileNamesSet.insert(fileName);
            }
            #ifdef DEBUG_PRNT
            std::cout << "File:"<<std::hex << std::setw(2) << std::setfill('0') << (0xFF & static_cast<unsigned char>(entry.DIR_Name[0])) << " ";
            std::cout << std::dec << std::endl;
            std::cout<<entry.DIR_Name<<std::endl;
            #endif

            fileCount++;
        }

        fileEntry.fileName = longFileName.empty() ? fileName : longFileName;
        fileEntry.firstCluster = (entry.DIR_FstClusHI << 16) | entry.DIR_FstClusLO; // перший кластер
        fileEntry.fileSize = entry.DIR_FileSize; // розмір файлу
        fileEntry.entryName = fileName;
        fileEntry.entryCluster = 0;

        printFileInfo32(fileEntry, entry);

        longFileName.clear();

        // Додаємо цей запис до списку файлів
        fileEntries.push_back(fileEntry);

    }
    std::cout<<"---------------------------------------------------------"<<std::endl;
    std::cout<<"Summary of instances found in the root directory:"<<std::endl;
    std::cout<<"Volume count: "<<volumeIDCount<<std::endl;
    std::cout<<"File count: "<<fileCount<<std::endl;
    std::cout<<"Directory count: "<<directoryCount<<std::endl;
    if (volumeIDCount == 0){
        std::cout<<"Warning! Missing volume directory entry. The disk image may be corrupted or created with a method that doesn't add the volume entry"<<std::endl;
        // isRootDirValid = false;
    }
    if (volumeIDCount > 1){
        std::cout<<"Error: Found too many volume directories!"<<std::endl;
        isRootDirValid = false;
    }

    if (isRootDirValid){
        std::cout<<"Finished analyzing the root directory. No errors found in the root directory."<<std::endl;
    }
    bool isfixed;
    if(fixErrors){
        // isfixed = fixRootDirErrors();
    }

    std::cout<<"---------------------------------------------------------"<<std::endl;

    if (isRootDirValid) {
        std::cout << "Root directory is valid." << std::endl;
    } else {
        std::cerr << "Errors detected in the root directory." << std::endl;
    }

    return isRootDirValid;
}
void detectClusterDuplication32(uint32_t* FAT, int FATSize, std::vector<FileEntry>& fileEntries, bool fixErrors) {
    std::unordered_map<int, std::vector<int>> clusterToFileMap;

    for (size_t fileIndex = 0; fileIndex < fileEntries.size(); ++fileIndex) {
        for (int cluster : fileEntries[fileIndex].clusterChain) {
            clusterToFileMap[cluster].push_back(fileIndex);
        }
    }

    // Перевірка на дублювання кластерів
    for (const auto& [cluster, fileIndices] : clusterToFileMap) {
        if (fileIndices.size() > 1) { // Якщо кластер використовується більше ніж одним файлом
            std::cout << "Error: Cluster " << cluster << " is used by multiple files:\n";
            for (int fileIndex : fileIndices) {
                std::cout << "  - File " << fileEntries[fileIndex].fileName << "\n";
            }
            std::cout << std::endl;

            if (fixErrors) {
                std::cout << "Do you want to fix this issue by marking EOF for affected files? (y/n): ";
                char response;
                std::cin >> response;
                if (response == 'y' || response == 'Y') {
                    // Залишаємо кластер для першого файлу, інші виправляємо
                    for (size_t i = 1; i < fileIndices.size(); ++i) {
                        int fileIndex = fileIndices[i];
                        auto& affectedFile = fileEntries[fileIndex];

                        // Оновлюємо передостанній кластер у ланцюжку
                        if (affectedFile.clusterChain.size() > 1) {
                            int secondLastCluster = affectedFile.clusterChain[affectedFile.clusterChain.size() - 2];
                            FAT[secondLastCluster] = 0xFFFFFFFF ; // EOF для FAT16
                            std::cout << "Updated cluster " << secondLastCluster
                                      << " to EOF (0xFFFF) for file " << affectedFile.fileName << "\n";

                            // Видаляємо останній кластер із ланцюжка
                            affectedFile.clusterChain.pop_back();
                        } else {
                            std::cout << "File " << affectedFile.fileName
                                      << " has no valid cluster chain to fix.\n";
                        }
                    }
                }
            }
        }
    }

}


// 2. Функція для перевірки невідповідності розміру файлу
void detectSizeMismatch32(const uint32_t* FAT, int FATSize, int bytesPerSec, int secPerCluster, std::vector<FileEntry>& fileEntries, bool fixErrors) {

    const int bytesPerCluster = bytesPerSec * secPerCluster;

    for (auto& entry : fileEntries) {
        // Отримуємо ланцюг кластерів для файлу

        // Підрахунок фактичного розміру файлу
        if(entry.clusterChain.empty()){
            continue;
        }
        int actualSize = (entry.clusterChain.size() - 1) * bytesPerCluster + (entry.fileSize % bytesPerCluster);

        // Перевірка на невідповідність розміру
        if (actualSize != entry.fileSize) {
            std::cout << "Warning: File " << entry.fileName << " size mismatch detected!\n";
            std::cout << "  Expected Size: " << entry.fileSize << " bytes\n";
            std::cout << "  Actual Size: " << actualSize << " bytes\n";
            // Пропонуємо користувачу вирішити проблему

            if (fixErrors) {
                std::cout << "Do you want to adjust the file size to match the actual size? (y/n): ";
                char response;
                std::cin >> response;
                if (response == 'y' || response == 'Y') {
                    entry.fileSize = actualSize;
                    std::cout << "File size updated to " << actualSize << " bytes for file " << entry.fileName << ".\n";
                }
            }

        }
    }
}

// 3. Функція для виявлення загублених кластерів та їх очищення
void detectAndFreeLostClusters32(uint32_t* FAT, int FATSize, const std::vector<FileEntry>& fileEntries, int rootCluster, bool fixErrors) {
    // Відстеження всіх кластерів, які використовуються файлами
    std::unordered_set<int> usedClusters;

    for (const auto& entry : fileEntries) {
        for (int cluster : entry.clusterChain) {
            usedClusters.insert(cluster);
        }
    }

    // Виявлення загублених кластерів
    for (int i = 2; i < FATSize; ++i) {
        if(i == rootCluster)
        {
            continue;
        }
        if (FAT[i] != 0x00000000 && usedClusters.find(i) == usedClusters.end()) {
            // Якщо кластер позначений як зайнятий, але не використовується, він вважається загубленим
            std::cout << "Lost cluster found at " << i << ". Marking it as free.\n";
            if (fixErrors) {
                FAT[i] = 0x00000000;
                std::cout << "Cluster " << i << " marked as free.\n";
            }
        }
    }
}

// 5. Функція для перевірки правильності EOC в кінці ланцюжка кластерів
void checkEndOfChain32(const uint32_t* FAT, int FATSize, const std::vector<FileEntry>& fileEntries, bool fixErrors) {
    for (const auto& entry : fileEntries) {
        if (entry.clusterChain.empty()){
            continue;
        }
        int lastCluster = entry.clusterChain.back();


        // Перевіряємо останній кластер на значення EOC у FAT
        if (FAT[lastCluster] < 0xFFFFFF8) {  // Некоректне закінчення
            std::cout << "Warning: File " << entry.fileName << " does not end with a correct EOC marker.\n";
            std::cout << "  Last cluster value in FAT: " << FAT[lastCluster] << " (Expected >= 0xFFF8)\n";

            if (fixErrors) {
                std::cout << "Do you want to fix the chain by marking the last cluster as EOC? (y/n): ";
                char response;
                std::cin >> response;
                if (response == 'y' || response == 'Y') {
//                    FAT[lastCluster] = 0xFFFF;  // Позначаємо останній кластер як EOC
                    std::cout << "The chain has been fixed for file " << entry.fileName << ".\n";
                }
            }
        } else {
#ifdef DEBUG_PRNT
            std::cout << "File " << entry.fileName << " ends with a correct EOC marker.\n";
#endif

        }
    }
}

std::vector<uint32_t > getClusterChain32(uint32_t startCluster, const uint32_t* FAT, int FATSize) {
    std::vector<uint32_t> chain;
    uint32_t cluster = startCluster;
#ifdef DEBUG_PRNT
    std::cout<<"Start cluster: "<< cluster<<std::endl;
#endif

    while (cluster < 0xFFFFFF8 && cluster > 1 && cluster < FATSize) {
        std::cout<<"Current Cluster: "<<cluster<<std::endl;
        if (std::find(chain.begin(), chain.end(), cluster) != chain.end()) {
            std::cout << "Cycle detected in cluster chain starting at cluster " << startCluster << ".\n";
            break;  // Виявлено зациклення
        }

        chain.push_back(cluster);
        cluster = FAT[cluster];
        std::cout<<"Next cluster: "<<cluster<<std::endl;

    }
    return chain;
}


void populateClusterChains32(const uint32_t* FAT, int FATSize, std::vector<FileEntry>& fileEntries) {
    for (auto& entry : fileEntries) {
        std::cout<<"entry: "<<entry.fileName<<std::endl;
        entry.clusterChain = getClusterChain32(entry.firstCluster, FAT, FATSize);
    }
}

// функція для аналізу кластера
void analyzeClusterInvariants32(uint32_t* FAT, int FATSize, int bytesPerSec, int secPerCluster, std::vector<FileEntry> &fileEntries, int rootCluster, bool fixErrors){
    // Зберігаємо ланцюги кластерів для кожного файлу
    populateClusterChains32(FAT, FATSize, fileEntries);

    // Виявлення дублікатів
    std::cout << "=== Detecting Cluster Duplication ===\n";
    detectClusterDuplication32(FAT, FATSize, fileEntries, fixErrors);
    std::cout << "=== Finished Detecting Cluster Duplication ===\n";
    //
    //    // Перевірка правильності завершення ланцюга кластерів
    std::cout << "\n=== Checking End of Chain Markers ===\n";
    checkEndOfChain32(FAT, FATSize, fileEntries, fixErrors);
    std::cout << "\n=== Finished Checking End of Chain Markers ===\n";
    //
    //    // Перевірка на відповідність розміру файлу
    std::cout << "\n=== Detecting Size Mismatch ===\n";
    detectSizeMismatch32(FAT, FATSize, bytesPerSec, secPerCluster, fileEntries, fixErrors);
    std::cout << "\n=== Finished Detecting Size Mismatch ===\n";
    ////
    ////    // Виявлення загублених кластерів
    std::cout << "\n=== Detecting and Freeing Lost Clusters ===\n";
    detectAndFreeLostClusters32(FAT, FATSize, fileEntries, rootCluster, fixErrors);
    std::cout << "\n=== Finished Detecting and Freeing Lost Clusters ====\n";
    ////

    std::cout << "\nAnalysis complete.\n";

};
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

void AnalyzeDiskData32(FILE *file, uint16_t bytesPerSec, uint8_t secPerClus, uint32_t dataStartSector, const std::vector<FAT32DirEntry> &dirEntries, std::vector<FileEntry> &fileEntries, uint32_t *FAT, uint32_t FATSize, bool fixErrors,  bool isRootDir) {


    uint32_t clusterNum;
    uint32_t startSectorAddress;
    std::vector<std::string> lfnParts;  // Для зберігання частин довгого імені
    std::string longFileName;
    std::vector<std::string> longFileNames;
    std::set<std::string> fileNamesSet; // Набір для перевірки дублікатів імен
    FileEntry fileEntry;
    bool isDiskDataValid = true;
    uint32_t entryCluster = 0;
    std::vector<uint32_t> visitedClusters;   // Для відстеження пройдених кластерів
    bool isDataValid = true;

    // Аналіз файлів і директорій
    for (const auto &entry : dirEntries) {
        std::string entryDirName(reinterpret_cast<const char *>(entry.DIR_Name), 11);
        entryDirName.erase(entryDirName.find_last_not_of(' ') + 1);  // Видаляємо пробіли в кінці імені
        // Ігнорувати записи, які мають ім'я "." або ".."
        if (entryDirName == "." || entryDirName == "..") {
            continue;
        }

        clusterNum = (entry.DIR_FstClusHI << 16) | entry.DIR_FstClusLO;
        startSectorAddress = dataStartSector + clusterNum*secPerClus;

        if (entry.DIR_Attr == 0x08){
            std::cout<<"Error: file and data region contains the volume entry"<<std::endl;
            isDiskDataValid = false;
            continue;
        }
        // Якщо це LFN запис, збираємо частини імені
        if (entry.DIR_Attr == 0x0F) {
            const auto* lfn = reinterpret_cast<const LFNEntry*>(&entry);
            lfnParts.push_back(extractLFN32(*lfn));
            continue;
        }

        // Якщо є частини довгого імені, об'єднуємо їх
        if (!lfnParts.empty()) {
            for (auto it = lfnParts.rbegin(); it != lfnParts.rend(); ++it) {
                longFileName += *it;
            }
            lfnParts.clear();  // Очищаємо частини імені після використання
            longFileNames.push_back(longFileName);
            // Перевірка імені
            if (!isValidName32(longFileName, 1)) {
                std::cout << "Error: Invalid long file name: " << longFileName << std::endl;
                isDiskDataValid = false;
            }

            // Перевірка на дублікати імен файлів
            if (fileNamesSet.find(longFileName) != fileNamesSet.end()) {
                std::cout << "Error: Duplicate name found: " << longFileName << std::endl;
                isDiskDataValid = false;
            } else {
                fileNamesSet.insert(longFileName);
            }


        }
        // Перевірка імені
        if (!isValidName32(entryDirName, 1)) {
            std::cout << "Error: Invalid long file name: " << longFileName << std::endl;
            isDiskDataValid = false;
        }

        // Перевірка на дублікати імен файлів
        if (fileNamesSet.find(entryDirName) != fileNamesSet.end()) {
            std::cout << "Error: Duplicate name found: " << longFileName << std::endl;
            isDiskDataValid = false;
        } else {
            fileNamesSet.insert(entryDirName);
        }
        if(!isRootDir){
            fileEntry.fileName = !longFileName.empty() ? longFileName : entryDirName;
            fileEntry.firstCluster = clusterNum;
            fileEntry.fileSize = entry.DIR_FileSize;
            fileEntry.entryName = entryDirName;
            fileEntry.entryCluster = entryCluster;
            fileEntries.push_back(fileEntry); // Додаємо запис у вектор всіх файлів
            printFileInfo32(fileEntry, entry);
        }
        entryCluster = clusterNum;
        longFileName.clear();


        // Якщо це файл, просто виводимо ім'я
        if (!(entry.DIR_Attr & 0x10)) { // Якщо це не директорія
            // std::cout << "File: " << entryDirName << std::endl;
            continue; // Не викликаємо рекурсію для файлів
        }

        visitedClusters.push_back(clusterNum);
                // Якщо це директорія, виводимо ім'я директорії та обробляємо її рекурсивно
#ifdef DEBUG_PRNT
        std::cout << "Directory: " << entryDirName << " (cluster #" << clusterNum << ")" << std::endl;
#endif
        // std::cout << "Directory: " << entryDirName << " contains the following entries:" << std::endl;
        std::vector<FAT32DirEntry> subDirEntries;
#ifdef DEBUG_PRNT
        std::cout << "Start sector address of cluster #" << clusterNum << ": " << startSectorAddress << std::endl;
#endif
        if (clusterNum >= 0xFFF8) { // Якщо кластер знаходиться в діапазоні зарезервованих або пошкоджених.
            std::cerr << "Error: Encountered a bad or reserved cluster pointer number: " << clusterNum << std::endl;
            isDiskDataValid = false;
            continue;
        }

        if (!readDataCluster32(file, bytesPerSec, clusterNum, secPerClus, dataStartSector, subDirEntries)) {
            std::cerr << "Failed to read cluster data for cluster #" << clusterNum << ".\n";
            isDataValid = false;
            continue;
        }else {
            // Виводимо результати
#ifdef DEBUG_PRNT
            std::cout << "Found entries in data region:" << std::endl;
            for (const auto &subEntry: subDirEntries) {
                std::cout << "  Entry: " << subEntry.DIR_Name << std::endl;
            }
#endif

            bool hasCurrentDir = false;
            bool hasParentDir = false;

            //Перевіряємо чи директорії . та .. взагалі існують
            for (const auto &subEntry: subDirEntries) {
                std::string subEntryName(reinterpret_cast<const char *>(subEntry.DIR_Name), 11);
                subEntryName.erase(subEntryName.find_last_not_of(' ') + 1);  // Видаляємо пробіли в кінці імені

                if (subEntryName == ".") {
                    // Перевіряємо, чи вказує "." на поточний кластер
                    uint32_t currentCluster = (subEntry.DIR_FstClusHI << 16) | subEntry.DIR_FstClusLO - 2;
                    if (currentCluster == clusterNum) {
                        hasCurrentDir = true;
                    }
                } else if (subEntryName == "..") {
                    // Перевіряємо, чи вказує ".." на батьківський кластер
                    uint32_t prevCluster = (subEntry.DIR_FstClusHI << 16) | subEntry.DIR_FstClusLO - 2;
                    hasParentDir = true;
                }
            }

            // Якщо не знайдено запис "." або "..", вивести попередження
            if (!hasCurrentDir) {
                std::cerr << "Warning: Directory does not contain the current directory entry ('.')." << std::endl;
            }
            if (!hasParentDir) {
                std::cerr << "Warning: Directory does not contain the parent directory entry ('..')." << std::endl;
            }
            AnalyzeDiskData32(file, bytesPerSec, secPerClus, dataStartSector, subDirEntries,  fileEntries, FAT, FATSize, fixErrors, false);

        }

    }

    // Перевірка загублених кластерів
    // std::unordered_set<uint32_t> usedClusters(visitedClusters.begin(), visitedClusters.end());
    // checkLostClusters(std::vector<uint32_t>(FAT, FAT + FATSize), FATSize, usedClusters, fixErrors);

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

bool isBootFAT32Invalid(extFAT32& bpb,  bool fixErrors){
    // функція перевіряє всі інваріанти бут сектора і повертає false якщо інформацію записано невірно і true якщо все добре
    bool isBootInvalid = false;

    BootSectorErrorsFAT32 bootSectorErrors;

    uint16_t bytsPerSec = bpb.basic.BPB_BytsPerSec;
    uint8_t secPerClus = bpb.basic.BPB_SecPerClus;
    uint8_t numFATs = bpb.basic.BPB_NumFATs;
    uint16_t rootEntCnt = bpb.basic.BPB_RootEntCnt;
    uint16_t rsvdSecCnt = bpb.basic.BPB_RsvdSecCnt;
    uint16_t totSec16 = bpb.basic.BPB_TotSec16;
    uint32_t totSec32 = bpb.basic.BPB_TotSec32;
    uint8_t media = bpb.basic.BPB_Media;
    uint16_t fatSize16 = bpb.basic.BPB_FATSz16;
    uint16_t fatSize32 = bpb.BPB_FATSz32;
    uint16_t secPerTrk = bpb.basic.BPB_SecPerTrk;
    uint16_t numHeads = bpb.basic.BPB_NumHeads;
    uint8_t drvNum = bpb.BS_DrvNum;
    uint8_t reserved1 = bpb.BS_Reserved1;
    uint8_t bootSig = bpb.BS_BootSig;
    uint32_t rootClusNum = bpb.BPB_RootClus;


    // Далі використовуємо ці змінні для перевірок
    const uint16_t validSectorSizes[] = {512, 1024, 2048, 4096};
    const uint8_t validSecPerClus[] = {1, 2, 4, 8, 16, 32, 64, 128};
    const uint8_t validMedia[] = {0xF0, 0xF8, 0xF9,
                                  0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF};


    // Перевірка BS_jmpBoot
    if (!(bpb.basic.BS_jmpBoot[0] == 0xEB && bpb.basic.BS_jmpBoot[2] == 0x90) && bpb.basic.BS_jmpBoot[0] != 0xE9){
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

    // перевіряємо к-сть зарезервованих секторів
    if (rsvdSecCnt == 0) {
        std::cerr << "Incorrect number of reserved sectors: " << rsvdSecCnt << std::endl;
        isBootInvalid = true;
    }

    // перевіряємо кількість FAT-таблиць
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
        std::cerr << "Warning! Invalid media type: "<< std::hex<<media<<std::dec<<std::endl;
        // isBootInvalid = true;
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
        std::cerr << "Warning! Invalid sectors per track: " << secPerTrk << std::endl;
        // isBootInvalid = true;
    }

    // Перевіряємо к-сть головок
    if (numHeads == 0) {
        std::cerr << "Warning! Invalid number of heads: " << numHeads << std::endl;
        // isBootInvalid = true;
    }

    // псеревіряємо загальну к-сть секторів
    if (totSec16 != 0) {
        std::cerr << "Warning! Invalid BPB_TotSec16, value here should be 0! Found: " << totSec16 << ". The image may have been created with less than recommended number of sectors"<<std::endl;
        // if (fixErrors) {
        //     std::cout << "Fixing: Setting BPB_TotSec16 to 0.\n";
        //     bpb->basic.BPB_TotSec16 = 0; // Виправлення
        // }
        // isBootInvalid = true;
    }
    if (totSec32 == 0) {
        std::cerr << "Number of sectors can't be zero! Found: " << totSec32 << std::endl;
        std::cerr<<"If value stored in TotSec16 isn't 0 will proceed with the number of sectors stored in that variable."<<std::endl;
        if(totSec16 != 0)
        {
            bpb.basic.BPB_TotSec32 = totSec16;
            totSec32 = totSec16;
        }
        else
        {
            std::cerr<<"Number of sectors can't be zero!"<<std::endl;
            isBootInvalid = true;
        }
        // if (fixErrors) {
        //     std::cout << "Fixing: Assigning a default value to BPB_TotSec32.\n";
        //     bpb->basic.BPB_TotSec32 = 100000;
        // }
        // isBootInvalid = true;
    }


    // Перевіряємо к-сть секторів для всіх FAT таблиць
    if (numFATs * fatSize32 >= totSec32) {
        std::cerr << "Number of sectors for FAT tables is equal or greater than number of all sectors. "
                  << "NumFATs: " << (int)numFATs << ", BPB_FATSz32: " << fatSize32 << ", TotalSectors: " << totSec32 << std::endl;
        if (fixErrors) {
            bpb.BPB_FATSz32 = totSec32 / (numFATs + 1);
            std::cout << "Fixing: Adjusting BPB_FATSz32 to: " << bpb.BPB_FATSz32 << std::endl;
        }
        isBootInvalid = true;
    }


    if (rootClusNum <= 1){
        std::cerr << " Invalid number of root cluster: "<< (int)rootClusNum<<std::endl;
        isBootInvalid = true;
    }

    // Перевіряємо BS_DrvNum
    if (drvNum != 0x80 && drvNum != 0x00) {
        std::cerr << " Warning! Invalid drive number: " << (int)drvNum << std::endl;
        // isBootInvalid = true;
    }

    // Перевіряємо BS_Reserved1
    if (reserved1 != 0x00) {
        std::cerr << "Warning! Invalid reserved field: " << static_cast<int>(reserved1) << std::endl;
        if (fixErrors) {
            std::cout << "Fixing reserved field to 0." << std::endl;
            bpb.BS_Reserved1 = 0x00; // Виправлення
        }
        // isBootInvalid = true;
    }


    // Перевіряємо BS_BootSig =  0x29 якщо один з наступних = 0x00
    if (bootSig == 0x29 && bpb.BS_VolID == 0x00 && bpb.BS_VolLab[0] == 0x00) {
        std::cerr << "Invalid boot signature: " << (int)bootSig << std::endl;
        isBootInvalid = true;
    }

    if(bpb.BS_BootSectorSig != 0xAA55)
    {
        std::cerr <<"Invalid signature word for FAT: "<< bpb.BS_BootSectorSig<<std::endl;
        isBootInvalid = true;
        bootSectorErrors.BPB_BootSignatureWord = true;
    }
    if (fixErrors && isBootInvalid) {
        handleInvalidBootSector32(bpb);
        isBootInvalid = false;
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

void handleInvalidBootSector32(extFAT32& bpb) {
    std::cout << "The boot sector is invalid." << std::endl;

    while (true) {
        std::cout << "Do you want to attempt restoration from backup? (yes/no): ";
        std::string choice;
        std::cin >> choice;

        if (choice == "yes" || choice == "y") {
            std::cout<<"---------------------------------------------------------"<<std::endl;
            std::cout << "Attempting to restore from backup..." << std::endl;

            // Спроба відновлення з резервної копії
            if (attemptRestoreFromBackup32(bpb)) {
                std::cout << "Backup restored successfully and is valid. Continuing execution..." << std::endl;
                if(!writeBootSectorToFile32(&bpb))
                {
                    std::cout<<"Failed to copy backup boot sector to the boot sector. Disk may be corrupted in the boot sector region. Will proceed with backup boot sector info."<<std::endl;
                }
                break; // Успішне відновлення, продовжити роботу
            } else {
                std::cerr << "Failed to restore from backup or restored boot sector is invalid." << std::endl;
                exit(EXIT_FAILURE); // Завершити програму
            }
        } else if (choice == "no" || choice == "n") {
            std::cout<<"---------------------------------------------------------"<<std::endl;
            std::cerr << "Cannot proceed without a valid boot sector. Exiting program." << std::endl;
            exit(EXIT_FAILURE); // Завершити програму
        } else {
            std::cerr << "Invalid choice. Please enter 'yes' or 'no'." << std::endl;
        }
    }
}

bool attemptRestoreFromBackup32(extFAT32& bpb) {
    if (!restoreFromBackup32(bpb)) {
        return false;
    }
    return !(isBootFAT32Invalid(bpb, false)); // Викликаємо перевірку валідності
}

bool restoreFromBackup32(extFAT32& bpb) {
    constexpr int bootSectorSize = sizeof(extFAT12_16);
    uint8_t backupBootSector[bootSectorSize];

    std::cout << "Attempting to restore boot sector from backup..." << std::endl;

    // Використовуємо readBackupBootSector для зчитування резервної копії
    if (!readBackupBootSector32(backupBootSector, bootSectorSize)) {
        std::cerr << "Failed to read backup boot sector." << std::endl;
        return false;
    }
    std::cout << "Successfully wrote boot sector" << std::endl;
    // std::cout << "Boot Sector Hex Dump:" << std::endl;
    // for (int i = 0; i < bootSectorSize; i++) {
    //     std::cout << std::hex << (int)backupBootSector[i] << " ";
    //     if ((i + 1) % 16 == 0) std::cout << std::endl; // Hex dump every 16 bytes
    // }
    // std::cout << std::dec;

    // Копіюємо дані з резервного сектора в структуру BPB
    memcpy(&bpb, backupBootSector, bootSectorSize);

    std::cout << "Boot sector successfully restored from backup. Checking if backup boot sector is valid." << std::endl;
    std::cout<<"---------------------------------------------------------"<<std::endl;
    return true;
}
