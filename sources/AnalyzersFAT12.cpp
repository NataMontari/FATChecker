
#include <iostream>
#include "FATstructs.hpp"
#include <algorithm>
#include <vector>
#include <fcntl.h>
#include <set>
#include <cstring>
#include <unordered_map>
#include "AnalyzersFAT12.hpp"
#include "readFunctions.hpp"
#include "FATTableStructs.hpp"
#include <set>
#include <unordered_set>

uint16_t readFAT12Entry(uint8_t* FAT, int entryIndex) {
    int byteIndex = entryIndex * 3 / 2;  // Кожен запис займає 1.5 байта
    int bitOffset = (entryIndex % 2) * 8; // Якщо парний кластер, то зчитуємо з першого байта, якщо непарний - з другого

    uint16_t entry = FAT[byteIndex] | (FAT[byteIndex + 1] << 8);  // Зчитуємо два байти

    if (entryIndex % 2 == 1) {
        entry >>= 4;  // Для непарних кластерів зсуваємо на 4 біти
    }

    return entry & 0xFFF;  // Повертаємо лише 12 біт
}

void writeFAT12Entry(uint8_t* FAT, int entryIndex, uint16_t value) {
    int byteIndex = entryIndex * 3 / 2;  // Кожен запис займає 1.5 байта

    // Якщо кластер парний, записуємо перші 8 біт в перший байт і останні 4 біти в другий
    if (entryIndex % 2 == 0) {
        FAT[byteIndex] = value & 0xFF;  // Менші 8 біт
        FAT[byteIndex + 1] = (value >> 8) & 0x0F;  // Старші 4 біти з 12-бітного значення
    } else {  // Якщо кластер непарний
        FAT[byteIndex] = value & 0x0F;  // Старші 4 біти з 12-бітного значення
        FAT[byteIndex + 1] = (value >> 4) & 0xFF;  // Менші 8 біт
    }
}

uint16_t getFAT12Entry(const uint8_t* FAT, int clusterIndex) {
    // Кожен FAT12 запис займає 12 біт, тобто 2 байти.
    int byteOffset = clusterIndex * 3 / 2;
    int bitOffset = (clusterIndex & 1) * 8;  // Для з'ясування, чи ми на парному або непарному індексі (по 12 біт на кластер).

    // Читаємо два байти для цього запису
    uint16_t entry;
    entry = (FAT[byteOffset] | (FAT[byteOffset + 1] << 8)) >> bitOffset;

    if (bitOffset == 8) {
        entry &= 0x0FFF; // Маскуємо зайві біти, щоб залишити тільки 12 біт.
    }

    return entry;
}

std::vector<uint32_t> getClusterChain12(uint32_t startCluster, const uint8_t* FAT, int FATSize) {
    std::vector<uint32_t> chain;
    uint32_t cluster = startCluster;

    while (cluster < 0xFFF8 && cluster > 1 && cluster < FATSize) {
        if (std::find(chain.begin(), chain.end(), cluster) != chain.end()) {
            std::cout << "Cycle detected in cluster chain starting at cluster " << startCluster << ".\n";
            break;  // Виявлено зациклення
        }
        chain.push_back(cluster);
        cluster = getFAT12Entry(FAT, cluster);
    }
    return chain;
}

void populateClusterChains12(FAT12& fat, std::vector<FileEntry>& fileEntries) {
    for (auto& file : fileEntries) {
        file.clusterChain.clear();
        int cluster = file.firstCluster;
        while (cluster != 0xFFF) { // Поки не досягли кінця ланцюга
            file.clusterChain.push_back(cluster);
            cluster = fat.getEntry(cluster);
        }
    }
}


void detectClusterDuplication12(FAT12& fat, std::vector<FileEntry>& fileEntries, bool fixErrors) {
    std::unordered_map<int, std::vector<int>> clusterToFileMap;

    for (size_t fileIndex = 0; fileIndex < fileEntries.size(); ++fileIndex) {
        for (int cluster : fileEntries[fileIndex].clusterChain) {
            clusterToFileMap[cluster].push_back(fileIndex);
        }
    }

    for (const auto& [cluster, fileIndices] : clusterToFileMap) {
        if (fileIndices.size() > 1) {
            std::cout << "Error: Cluster " << cluster << " is used by multiple files:\n";
            for (int fileIndex : fileIndices) {
                std::cout << "  - File " << fileEntries[fileIndex].fileName << "\n";
            }

            if (fixErrors) {
                std::cout << "Fixing duplication by marking EOF for extra files.\n";
                for (size_t i = 1; i < fileIndices.size(); ++i) {
                    int fileIndex = fileIndices[i];
                    auto& affectedFile = fileEntries[fileIndex];
                    int lastCluster = affectedFile.clusterChain.back();
                    fat.setEntry(lastCluster, 0xFFF);
                    affectedFile.clusterChain.pop_back();
                }
            }
        }
    }
}

void detectSizeMismatch12(const uint8_t* FAT, int FATSize, int bytesPerSec, int secPerCluster, std::vector<FileEntry>& fileEntries, bool fixErrors) {
    const int bytesPerCluster = bytesPerSec * secPerCluster;

    for (auto& entry : fileEntries) {
        if(entry.clusterChain.empty()) continue;

        int actualSize = (entry.clusterChain.size() - 1) * bytesPerCluster + (entry.fileSize % bytesPerCluster);

        if (actualSize != entry.fileSize) {
            std::cout << "Warning: File " << entry.fileName << " size mismatch detected!\n";
            std::cout << "  Expected Size: " << entry.fileSize << " bytes\n";
            std::cout << "  Actual Size: " << actualSize << " bytes\n";

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


void checkEndOfChain12(FAT12& fat, std::vector<FileEntry>& fileEntries, bool fixErrors) {
    for (auto& file : fileEntries) {
        int lastCluster = file.clusterChain.back();
        uint16_t entry = fat.getEntry(lastCluster);
        if (entry != 0xFFF) {
            std::cout << "Error: File " << file.fileName << " does not end with EOF.\n";

            if (fixErrors) {
                fat.setEntry(lastCluster, 0xFFF);
                std::cout << "Fixed: Marked cluster " << lastCluster << " as EOF.\n";
            }
        }
    }
}
void detectAndFreeLostClusters12(FAT12& fat, std::vector<FileEntry>& fileEntries, bool fixErrors) {
    std::unordered_set<int> usedClusters;
    for (const auto& file : fileEntries) {
        usedClusters.insert(file.clusterChain.begin(), file.clusterChain.end());
    }

    for (int cluster = 2; cluster < fat.FATSize / 3 * 2; ++cluster) {
        if (fat.getEntry(cluster) != 0 && fat.getEntry(cluster) != 0xFFF && !usedClusters.count(cluster)) {
            std::cout << "Lost cluster detected: " << cluster << "\n";

            if (fixErrors) {
                fat.freeCluster(cluster);
                std::cout << "Freed cluster " << cluster << "\n";
            }
        }
    }
}


void analyzeClusterInvariants12(uint8_t* &FAT, int FATSize, int bytesPerSec, int secPerCluster, std::vector<FileEntry>& fileEntries, bool fixErrors) {
    // Зберігаємо ланцюги кластерів для кожного файлу

    std::cout<<"Analyzing cluster invariants"<<std::endl;
    std::cout<<"---------------------------------------------------------"<<std::endl;
    FAT12 FATTable(FAT, FATSize);
    populateClusterChains12(FATTable, fileEntries);

    // Виявлення дублікатів
    std::cout << "=== Detecting Cluster Duplication ===\n";
    detectClusterDuplication12(FATTable, fileEntries, fixErrors);

    // Перевірка правильності завершення ланцюга кластерів
    std::cout << "\n=== Checking End of Chain Markers ===\n";
    checkEndOfChain12(FATTable, fileEntries, fixErrors);

    // Перевірка на відповідність розміру файлу
    std::cout << "\n=== Detecting Size Mismatch ===\n";
    detectSizeMismatch12(FAT, FATSize, bytesPerSec, secPerCluster, fileEntries, fixErrors);

    // Виявлення загублених кластерів
    std::cout << "\n=== Detecting and Freeing Lost Clusters ===\n";
    detectAndFreeLostClusters12(FATTable, fileEntries, fixErrors);

    std::cout << "\nAnalysis complete.\n";
}

bool analyzeFAT12Tables(const std::vector<uint8_t*>& FATs, int FATSize, uint16_t bytesPerSec, int startFATSector, bool fixErrors) {
#ifdef DEBUG_PRNT
    std::cout << "---------------------------------" << std::endl;
    std::cout << "Trying to analyze FAT12 tables" << std::endl;
#endif

    bool fixed = false;
    if (FATs.empty()) {
        std::cout << "FAT tables weren't found" << std::endl;
        return false;
    }

    size_t numFATs = FATs.size();

    for (size_t i = 1; i < numFATs; i++) {
        // Порівняння FAT таблиць
        if (std::memcmp(FATs[0], FATs[i], FATSize * bytesPerSec) != 0) {
            std::cout << "FAT12 table " << i + 1 << " differs from FAT12 table 1." << std::endl;
            if (fixErrors) {
                // Виправлення таблиці FAT12, копіюючи дані з першої таблиці
                std::memcpy(FATs[i], FATs[0], FATSize * bytesPerSec);
                writeFATTableToFile12(FATs[0], i, bytesPerSec, FATSize, startFATSector);
                std::cout << "FAT12 table " << i + 1 << " has been fixed by copying from FAT12 table 1." << std::endl;
                fixed = true;
            } else {
                return false; // Якщо fixErrors == false, повертаємо false
            }
        }
    }

    if (!fixed) {
        std::cout << "All FAT12 tables match." << std::endl;
    }

    return true; // Усі таблиці співпадають
}







bool isValidName12(const std::string& name, int longNameFlag) {
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
std::string extractLFN12(const LFNEntry& entry) {
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

// Основна функція для отримання довгого імені з FAT16 директорії
std::string getLongFileName12(const std::vector<FAT16DirEntry>& rootDirEntries) {
    std::string longName;
    std::vector<std::string> parts; // Для зберігання частин довгого імені

    size_t entrySize = sizeof(FAT16DirEntry);  // Розмір одного запису (32 байти)

    for (const auto &entry: rootDirEntries) {
        // Перевірка, чи це LFN запис (атрибут = 0x0F)
        if (static_cast<unsigned char>(entry.DIR_Name[0]) == 0xE5){
            continue;
        }
        if (entry.DIR_Attr == 0x0F) {
            const auto* lfn = reinterpret_cast<const LFNEntry*>(&entry);
            parts.push_back(extractLFN12(*lfn));
        }
            // Якщо це основний запис файлу
        else if (!parts.empty()) {
            // Склеїмо всі частини у правильному порядку
            for (auto it = parts.rbegin(); it != parts.rend(); ++it) {
                longName += *it;
            }
            std::cout<<longName<<std::endl;
            parts.clear();
            longName="";
            //break;  // Завершуємо, коли знайшли основний запис
        }
    }

    return longName;
}

bool check_date12(uint16_t date_value) {
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

bool check_time12(uint16_t time_value) {
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

bool checkEntry12(const FAT12DirEntry& entry){
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
    if (!check_date12(entry.DIR_CrtDate)) {
        std::cout << "Error: Invalid creation date." << std::endl;
        isEntryValid = false;
    }

    // Validate last access date
    if (!check_date12(entry.DIR_LstAccDate)) {
        std::cout << "Error: Invalid last access date." << std::endl;
        isEntryValid = false;
    }

    // Validate write date
    if (!check_date12(entry.DIR_WrtDate)) {
        std::cout << "Error: Invalid write date." << std::endl;
        isEntryValid = false;
    }

    // Validate creation time
    if (!check_time12(entry.DIR_CrtTime)) {
        std::cout << "Error: Invalid creation time." << std::endl;
        isEntryValid = false;
    }

    // Validate write time
    if (!check_time12(entry.DIR_WrtTime)) {
        std::cout << "Error: Invalid write time." << std::endl;
        isEntryValid = false;
    }


    return isEntryValid;
}

bool isClusterValid12(uint16_t cluster, uint16_t maxCluster) {
    // Перевірка на заборонені значення для FAT12
    return !(cluster == 0 || cluster == 1 ||
             (cluster >= 0xFF7 && cluster <= 0xFFF) || // Враховуємо діапазон кластерів для FAT12
             (cluster > maxCluster && cluster <= 0xFF6));
}

void printFileInfo12( FileEntry fileEntry, FAT12DirEntry entry)
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


bool AnalyzeRootDir12(std::vector<FAT12DirEntry>& rootDirEntries, std::vector<FAT12DirEntry>& dataDirEntries,  std::vector<FileEntry>& fileEntries, bool fixErrors){
    bool isRootDirValid = true;
    int volumeIDCount = 0;
    int directoryCount = 0;
    int fileCount = 0;
    std::vector<std::string> lfnParts;  // Для зберігання частин довгого імені
    std::string longFileName;
    std::vector<std::string> longFileNames;
    std::set<std::string> fileNamesSet; // Набір для перевірки дублікатів імен
    FileEntry fileEntry;

    std::cout<<"---------------------------------------------------------"<<std::endl;

    std::cout<<"Analyzing root directory"<<std::endl;

    for(auto &entry: rootDirEntries){

#ifdef DEBUG_PRNT
        // Виводимо кожен символ у шістнадцятковому форматі
        for (int i = 0; i < 1; i++) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (0xFF & static_cast<unsigned char>(entry.DIR_Name[i])) << " ";
        }

        std::cout << std::dec << std::endl;  // Повертаємося до десяткового формату

#endif
        if(entry.DIR_Name[0] == 0){
            break;
        }

        std::string entryName(reinterpret_cast<const char*>(entry.DIR_Name), 11);
        entryName = entryName.substr(0, entryName.find(' '));

        if (entryName == ".") {
            std::cout << "Error: Root directory contains a dot (.) entry." << std::endl;
            isRootDirValid = false;

            if (fixErrors) {
                uint16_t cluster = ((entry.DIR_FstClusHI & 0x0F) << 8) | entry.DIR_FstClusLO;
                if (isClusterValid12(cluster, 0xFF5)) {
                    std::cout << "Entry points to a valid cluster. Would you like to rename it? (y/n): ";
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
                uint16_t cluster = ((entry.DIR_FstClusHI & 0x0F) << 8) | entry.DIR_FstClusLO;
                if (isClusterValid12(cluster, 0xFF5)) {
                    std::cout << "Entry points to a valid cluster. Would you like to rename it? (y/n): ";
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

        if (static_cast<unsigned char>(entry.DIR_Name[0]) == 0xe5) { // Якщо ім'я не пуста стрічка
            continue;
        }

        // Якщо це LFN запис (атрибут = 0x0F), збираємо довге ім'я
        if (entry.DIR_Attr == 0x0F) {
            const auto* lfn = reinterpret_cast<const LFNEntry*>(&entry);
            lfnParts.push_back(extractLFN12(*lfn));
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
            if (!isValidName12(longFileName, 1)) {
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

        if (!isValidName12(fileName, 0)) {
            std::cout << "Error: Invalid short file name: " << fileName << std::endl;

        }

        // Перевірка на коректність атрибутів
        if ((entry.DIR_Attr & 0x3F) == 0) { // Атрибути не повинні мати недопустимі значення
            std::cout << "Error: Invalid attribute detected for entry." << std::endl;
            isRootDirValid = false;

        }

        if(!checkEntry12(entry)){
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
                std::cout<<"Error: root entry shouldn't have any associated time stamps"<<std::endl;
                isRootDirValid = false;
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
        fileEntry.firstCluster = ((entry.DIR_FstClusHI & 0x0F) << 8) | entry.DIR_FstClusLO; // перший кластер
        fileEntry.fileSize = entry.DIR_FileSize; // розмір файлу
        fileEntry.entryName = fileName;
        fileEntry.entryCluster = 0;
        printFileInfo12(fileEntry, entry);

        longFileName.clear();

        // Додаємо цей запис до списку файлів
        fileEntries.push_back(fileEntry);

    }

    std::cout<<"---------------------------------------------------------"<<std::endl;
#ifdef DEBUG_PRNT
    std::cout<<"Program found the following files with long names in the root directory:"<<std::endl;
    for (const auto &name: longFileNames){
        std::cout<<"Long file name: "<<name<<std::endl;
    }
#endif
    //    std::cout << "Long file name: " << longFileName << std::endl;

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
    return isRootDirValid;
};


bool readDataCluster(FILE *file, uint16_t bytesPerSec, uint32_t startSectorAdress, uint8_t secPerClus, std::vector<FAT12DirEntry> & subDirEntries) {
    const int ENTRY_SIZE = sizeof(FAT12DirEntry); // Розмір одного запису в байтах
    const int SECTOR_SIZE = bytesPerSec; // Розмір сектора в байтах
    uint8_t buffer[SECTOR_SIZE * secPerClus]; // Буфер для читання секторів
    uint64_t address = static_cast<uint64_t>(startSectorAdress) * SECTOR_SIZE;
    // Читаємо сектори даних
#ifdef DEBUG_PRNT
    std::cout<<"Physical address of cluster: "<< address<<std::endl;
#endif

    if (fseek(file, address, SEEK_SET) < 0) {
        std::cerr << "Failed to seek to data start sector." << std::endl;
        return false;
    }

    ssize_t bytesRead = fread( buffer, 1, sizeof(buffer), file);
    if (bytesRead < 0 || bytesRead != SECTOR_SIZE * secPerClus) {
        std::cerr << "Failed to read data section." << std::endl;
        return false;
    }

    // Обробка прочитаних даних
    for (uint32_t i = 0; i < bytesRead / ENTRY_SIZE; ++i) {
        FAT12DirEntry entry;
        std::memcpy(&entry, &buffer[i * ENTRY_SIZE], ENTRY_SIZE); // Копіюємо запис

        // Перевіряємо, чи є запис дійсним
        if (entry.DIR_Name[0] == 0) {
            // Це кінець записів у кластері
            break;
        }

        if (static_cast<unsigned char> (entry.DIR_Name[0]) == 0xE5) {
            // Запис є видаленим
            continue;
        }

        // Перевірка атрибута
        if (entry.DIR_Attr & 0x04) {
            // Якщо атрибут 0x08, то це системний файл
            std::cerr << "Warning: System file detected: " << entry.DIR_Name << std::endl;
        }


//        std::string entryDirName(reinterpret_cast<const char*>(entry.DIR_Name), 11);
//        entryDirName.erase(entryDirName.find_last_not_of(' ') + 1);  // Видаляємо пробіли в кінці імені

//        if (entryDirName == "."){
//            std::cout<<"found the cur dir entry"<<std::endl;
//            continue;
//        }
//        if(entryDirName == ".."){
//            std::cout<<"found the parent dir entry"<<std::endl;
//            continue;
//        }

        // Додаємо запис у вектор
        subDirEntries.push_back(entry);
    }

    return true;
}

bool AnalyzeDiskData12(FILE *file, uint16_t bytesPerSec, uint8_t sectorsPerCluster, uint32_t dataStartSector, const std::vector<FAT12DirEntry>& dataDirEntries, std::vector<FileEntry> &fileEntries, std::set<uint32_t>& processedClusters, bool fixErrors, bool isRootDir){
    uint32_t clusterNum;
    uint32_t startSectorAddress;
    std::vector<std::string> lfnParts;  // Для зберігання частин довгого імені
    std::string longFileName;
    std::vector<std::string> longFileNames;
    std::set<std::string> fileNamesSet; // Набір для перевірки дублікатів імен
    FileEntry fileEntry;
    bool isDiskDataValid = true;
    uint32_t entryCluster = 0;

//    std::cout<<dataStartSector<<std::endl;
    for (const auto & entry: dataDirEntries) {
        std::string entryDirName(reinterpret_cast<const char *>(entry.DIR_Name), 11);
        entryDirName.erase(entryDirName.find_last_not_of(' ') + 1);  // Видаляємо пробіли в кінці імені
        // Ігнорувати записи, які мають ім'я "." або ".."
        if (entryDirName == "." || entryDirName == "..") {
            continue;
        }

        clusterNum = ((entry.DIR_FstClusHI & 0x0F) << 8) | entry.DIR_FstClusLO - 2;
        startSectorAddress = dataStartSector + clusterNum*sectorsPerCluster;


        if (entry.DIR_Attr == 0x08){
            std::cout<<"Error: file and data region contains the volume entry"<<std::endl;
            isDiskDataValid = false;
            continue;
        }
        // Якщо це LFN запис, збираємо частини імені
        if (entry.DIR_Attr == 0x0F) {
            const auto* lfn = reinterpret_cast<const LFNEntry*>(&entry);
            lfnParts.push_back(extractLFN12(*lfn));
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
            if (!isValidName12(longFileName, 1)) {
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
        if (!isValidName12(entryDirName, 1)) {
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
            fileEntry.firstCluster = clusterNum + 2;
            fileEntry.fileSize = entry.DIR_FileSize;
            fileEntry.entryName = entryDirName;
            fileEntry.entryCluster = entryCluster;
            fileEntries.push_back(fileEntry); // Додаємо запис у вектор всіх файлів
            printFileInfo12(fileEntry, entry);

        }

        entryCluster = clusterNum;
        longFileName.clear();


        // Якщо це файл, просто виводимо ім'я
        if (!(entry.DIR_Attr & 0x10)) { // Якщо це не директорія
            // std::cout << "File: " << entryDirName << std::endl;
            continue; // Не викликаємо рекурсію для файлів
        }
        if (processedClusters.find(clusterNum) != processedClusters.end()) {
            continue;
        }
        processedClusters.insert(clusterNum);
        // Якщо це директорія, виводимо ім'я директорії та обробляємо її рекурсивно
#ifdef DEBUG_PRNT
        std::cout << "Directory: " << entryDirName << " (cluster #" << clusterNum << ")" << std::endl;
#endif
        std::vector<FAT12DirEntry> subDirEntries;

#ifdef DEBUG_PRNT
        std::cout << "Start sector address of cluster #" << clusterNum << ": " << startSectorAddress << std::endl;
#endif

        if (clusterNum >= 0xFF8) { // Якщо кластер знаходиться в діапазоні зарезервованих або пошкоджених.
            std::cerr << "Error: Found a bad or reserved cluster pointer number: " << clusterNum << std::endl;
            isDiskDataValid = false;
            continue;
        }

        if (!readDataCluster(file, bytesPerSec, startSectorAddress, sectorsPerCluster, subDirEntries)) {
            std::cout << "Failed to read entries from the data region." << std::endl;
        } else {
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
                    uint32_t currentCluster = ((subEntry.DIR_FstClusHI & 0x0F) << 8) | subEntry.DIR_FstClusLO - 2;
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

            AnalyzeDiskData12(file, bytesPerSec, sectorsPerCluster, dataStartSector, subDirEntries,  fileEntries, processedClusters, fixErrors, false);

        }
    }
    bool isFixed;
    if(fixErrors){
        isFixed = fixDataRegionErrors();
    }
#ifdef DEBUG_PRNT
    std::cout<<"Program found the following files with long names in the root directory:"<<std::endl;
    for (const auto &name: longFileNames){
        std::cout<<"Long file name: "<<name<<std::endl;
    }
    std::cout << "File and folder verification is complete." << std::endl;
#endif

    return isDiskDataValid;
}

// Перевірка, чи значення входить у допустимий список
template <typename T, size_t N>
bool isValid(T value, const T (&validArray)[N]) {
    return std::any_of(std::begin(validArray), std::end(validArray), [value](T v) { return v == value; });
}

bool isBootFAT12Invalid(extFAT12_16* bpb, bool fixErrors){
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


    // Константні змінні для перевірок
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

    // Перевіряємо к-сть FAT-таблиць
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

    // Перевіряємо загальну к-сть секторів
    if (totSec32 < 65536 && totSec16 == 0){
        std::cerr << "Number of sectors can't be zero!" << std::endl;
        isBootInvalid = true;
    }
    if (totSec16 > 4084){
        std::cerr << "FAT12 can't contain more than 4084 sectors! "<<std::endl;
        isBootInvalid = true;
    }

    // Перевіряємо к-сть секторів для всіх FAT таблиць
    int numOfSecsForFAT = numFATs * fatSize;
    if ((totSec32 != 0 && numOfSecsForFAT >= totSec32) || (totSec16 != 0 && numOfSecsForFAT >= totSec16)){
        std::cerr << "Number of sectors for FAT tables is equal or greater than number of all sectors"<<std::endl;
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
//        isBootInvalid = true;
    }

    // Перевіряємо BS_BootSig =  0x29 якщо один з наступних = 0x00
    if (bootSig == 0x29 && bpb->BS_VolID == 0x00 && bpb->BS_VolLab[0] == 0x00) {
        std::cerr << "Invalid boot signature: " << (int)bootSig << std::endl;
        isBootInvalid = true;
    }

    if (fixErrors && isBootInvalid) {
        handleInvalidBootSector12(bpb);
        isBootInvalid = false;
    }
    // (Потім) Перевірка Signature_word

    // (Потім) Перевірка останнього біта - 0x00 якщо BPB_BytsPerSec > 512

    return isBootInvalid;
};


void handleInvalidBootSector12(extFAT12_16* bpb) {
    std::cout << "The boot sector is invalid." << std::endl;

    while (true) {
        std::cout << "Do you want to attempt restoration from backup? (yes/no): ";
        std::string choice;
        std::cin >> choice;

        if (choice == "yes" || choice == "y") {
            std::cout << "Attempting to restore from backup..." << std::endl;

            // Спроба відновлення з резервної копії
            if (attemptRestoreFromBackup12(bpb)) {
                std::cout << "Backup restored successfully and is valid. Continuing execution..." << std::endl;
                break; // Успішне відновлення, продовжити роботу
            } else {
                std::cerr << "Failed to restore from backup or restored boot sector is invalid." << std::endl;
                exit(EXIT_FAILURE); // Завершити програму
            }
        } else if (choice == "no" || choice == "n") {
            std::cerr << "Cannot proceed without a valid boot sector. Exiting program." << std::endl;
            exit(EXIT_FAILURE); // Завершити програму
        } else {
            std::cerr << "Invalid choice. Please enter 'yes' or 'no'." << std::endl;
        }
    }
}

bool attemptRestoreFromBackup12(extFAT12_16* bpb) {
    if (!restoreFromBackup12(bpb)) {
        return false;
    }
    return !(isBootFAT12Invalid(bpb, false)); // Викликаємо перевірку валідності
}

bool restoreFromBackup12(extFAT12_16* bpb) {
    constexpr int bootSectorSize = sizeof(extFAT12_16);
    uint8_t backupBootSector[bootSectorSize];

    std::cout << "Attempting to restore boot sector from backup..." << std::endl;

    // Використовуємо readBackupBootSector для зчитування резервної копії
    if (!readBackupBootSector(backupBootSector, bootSectorSize)) {
        std::cerr << "Failed to read backup boot sector." << std::endl;
        return false;
    }

    // Копіюємо дані з резервного сектора в структуру BPB
    memcpy(bpb, backupBootSector, bootSectorSize);

    std::cout << "Boot sector successfully restored from backup." << std::endl;
    return true;
}
