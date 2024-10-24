
#include <iostream>
#include "FATstructs.hpp"
#include <algorithm>
#include <vector>
#include <cstring>
#include <iomanip>
#include <stdio.h>
#include <fcntl.h>
#include <set>


//#define DEBUG_PRNT


// Аналіз та порівняння FAT таблиць
bool analyzeFAT16Tables(const std::vector<uint16_t*>& FATs, int FATSize, uint16_t bytesPerSec) {
    #ifdef DEBUG_PRNT
    std::cout<< "---------------------------------"<<std::endl;
    std::cout<<"Trying to analyze FAT tables\n"<<std::endl;
    #endif
    if (FATs.empty()) {
        std::cout<<"FAT tables weren't found"<<std::endl;
        return false;
    }

    size_t numFATs = FATs.size();
    for (size_t i = 1; i < numFATs; i++) {
        // Порівняння FAT таблиць
        if (std::memcmp(FATs[0], FATs[i], sizeof(uint16_t) * (FATSize * bytesPerSec / sizeof(uint16_t))) != 0) {
            std::cout << "FAT table " << i + 1 << " differs from FAT table 1." << std::endl;
            return false; // Якщо хоча б одна таблиця не співпадає
        }
    }
    std::cout << "All FAT tables match." << std::endl;
    return true; // Усі таблиці співпадають
}



void AnalyzeCopyFAT16(){
    std::cout<<"Analyzing Copy FAT Table"<<std::endl;
};

#include <cctype>  // Для std::isalnum

bool isValidName(const std::string& name, int longNameFlag) {
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
std::string extractLFN(const LFNEntry& entry) {
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
std::string getLongFileName(const std::vector<FAT16DirEntry>& rootDirEntries) {
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
            parts.push_back(extractLFN(*lfn));
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

bool check_time(uint16_t time_value) {
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

bool checkEntry(const FAT16DirEntry& entry){
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
    if (!check_date(entry.DIR_CrtDate)) {
        std::cout << "Error: Invalid creation date." << std::endl;
        isEntryValid = false;
    }

    // Validate last access date
    if (!check_date(entry.DIR_LstAccDate)) {
        std::cout << "Error: Invalid last access date." << std::endl;
        isEntryValid = false;
    }

    // Validate write date
    if (!check_date(entry.DIR_WrtDate)) {
        std::cout << "Error: Invalid write date." << std::endl;
        isEntryValid = false;
    }

    // Validate creation time
    if (!check_time(entry.DIR_CrtTime)) {
        std::cout << "Error: Invalid creation time." << std::endl;
        isEntryValid = false;
    }

    // Validate write time
    if (!check_time(entry.DIR_WrtTime)) {
        std::cout << "Error: Invalid write time." << std::endl;
        isEntryValid = false;
    }


    return isEntryValid;
}

bool AnalyzeRootDir16(const std::vector<FAT16DirEntry>& rootDirEntries, std::vector<FAT16DirEntry>& dataDirEntries){
    bool isRootDirValid = true;
    int volumeIDCount = 0;
    int directoryCount = 0;
    int fileCount = 0;
    std::vector<std::string> lfnParts;  // Для зберігання частин довгого імені
    std::string longFileName;
    std::vector<std::string> longFileNames;
    std::set<std::string> fileNamesSet; // Набір для перевірки дублікатів імен


    for(const auto &entry: rootDirEntries){

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

        if(static_cast<unsigned char>(entry.DIR_Name[0])==0x2e){
            if(static_cast<unsigned char>(entry.DIR_Name[1]) == 0x2e){
                std::cout<<"Error: Root directory contains dotdot(..) entry"<<std::endl;
                isRootDirValid = false;
                continue;
            }else{
                std::cout<<"Error: Root directory contains dot(.) entry"<<std::endl;
                isRootDirValid = false;
                continue;
            }
        }


        if (static_cast<unsigned char>(entry.DIR_Name[0]) == 0xe5) { // Якщо ім'я не пуста стрічка
            continue;
        }

        // Якщо це LFN запис (атрибут = 0x0F), збираємо довге ім'я
        if (entry.DIR_Attr == 0x0F) {
            const auto* lfn = reinterpret_cast<const LFNEntry*>(&entry);
            lfnParts.push_back(extractLFN(*lfn));
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
            if (!isValidName(longFileName, 1)) {
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
            longFileName.clear();
        }

        // Перевірка короткого імені
        std::string fileName(entry.DIR_Name, 11);

        if (!isValidName(fileName, 0)) {
            std::cout << "Error: Invalid short file name: " << fileName << std::endl;
            isRootDirValid = false;
        }

        // Перевірка на коректність атрибутів
        if ((entry.DIR_Attr & 0x3F) == 0) { // Атрибути не повинні мати недопустимі значення
            std::cout << "Error: Invalid attribute detected for entry." << std::endl;
            isRootDirValid = false;
        }

        if(!checkEntry(entry)){
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


    }

    std::cout<<"Program found the following files with long names in the root directory:"<<std::endl;
    for (const auto &name: longFileNames){
        std::cout<<"Long file name: "<<name<<std::endl;
    }
//    std::cout << "Long file name: " << longFileName << std::endl;

    std::cout<<"Instances found in the root directory:"<<std::endl;
    std::cout<<"Volume count: "<<volumeIDCount<<std::endl;
    std::cout<<"File count: "<<fileCount<<std::endl;
    std::cout<<"Directory count: "<<directoryCount<<std::endl;
    if (volumeIDCount == 0){
        std::cout<<"Missing volume directory entry"<<std::endl;
        isRootDirValid = false;
    }
    if (volumeIDCount > 1){
        std::cout<<"Found too many volume directories!"<<std::endl;
        isRootDirValid = false;
    }

    if (isRootDirValid){
        std::cout<<"No errors found in the root directory."<<std::endl;
    }

    return isRootDirValid;
};

bool readDataCluster(int fd, uint16_t bytesPerSec, uint32_t startSectorAdress, uint8_t secPerClus, std::vector<FAT16DirEntry> & subDirEntries) {
    const int ENTRY_SIZE = sizeof(FAT16DirEntry); // Розмір одного запису в байтах
    const int SECTOR_SIZE = bytesPerSec; // Розмір сектора в байтах
    uint8_t buffer[SECTOR_SIZE * secPerClus]; // Буфер для читання секторів
    uint64_t address = static_cast<uint64_t>(startSectorAdress) * SECTOR_SIZE;
    // Читаємо сектори даних
    std::cout<<"Physical address of cluster: "<< address<<std::endl;
    if (lseek(fd, address, SEEK_SET) < 0) {
        std::cerr << "Failed to seek to data start sector." << std::endl;
        return false;
    }

    ssize_t bytesRead = read(fd, buffer, sizeof(buffer));
    if (bytesRead < 0 || bytesRead != SECTOR_SIZE * secPerClus) {
        std::cerr << "Failed to read data section." << std::endl;
        return false;
    }

    // Обробка прочитаних даних
    for (uint32_t i = 0; i < bytesRead / ENTRY_SIZE; ++i) {
        FAT16DirEntry entry;
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
            continue;
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

bool AnalyzeDiskData16(int fd, uint16_t bytesPerSec, uint8_t sectorsPerCluster, uint32_t dataStartSector, const std::vector<FAT16DirEntry>& dataDirEntries) {
    std::cout << "The program is verifying files and folders..." << std::endl;
    uint32_t clusterNum;
    uint32_t startSectorAddress;
    std::vector<std::string> lfnParts;  // Для зберігання частин довгого імені
    std::string longFileName;
    std::vector<std::string> longFileNames;
    std::set<std::string> fileNamesSet; // Набір для перевірки дублікатів імен
    bool isDiskDataValid = true;

//    std::cout<<dataStartSector<<std::endl;
    for (const auto & entry: dataDirEntries) {
        std::string entryDirName(reinterpret_cast<const char *>(entry.DIR_Name), 11);
        entryDirName.erase(entryDirName.find_last_not_of(' ') + 1);  // Видаляємо пробіли в кінці імені
        // Ігнорувати записи, які мають ім'я "." або ".."
        if (entryDirName == "." || entryDirName == "..") {
            continue;
        }

        clusterNum = (entry.DIR_FstClusHI << 16) | entry.DIR_FstClusLO - 2;
        startSectorAddress = dataStartSector + clusterNum;


        if (entry.DIR_Attr == 0x08){
            std::cout<<"Error: file and data region contains the volume entry"<<std::endl;
            isDiskDataValid = false;
            continue;
        }
        // Якщо це LFN запис, збираємо частини імені
        if (entry.DIR_Attr == 0x0F) {
            const auto* lfn = reinterpret_cast<const LFNEntry*>(&entry);
            lfnParts.push_back(extractLFN(*lfn));
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
            if (!isValidName(longFileName, 1)) {
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
            longFileName.clear();

        }

        // Перевірка імені
        if (!isValidName(entryDirName, 1)) {
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

        // Якщо це файл, просто виводимо ім'я
        if (!(entry.DIR_Attr & 0x10)) { // Якщо це не директорія
            std::cout << "File: " << entryDirName << std::endl;
            continue; // Не викликаємо рекурсію для файлів
        }

        // Якщо це директорія, виводимо ім'я директорії та обробляємо її рекурсивно
        std::cout << "Directory: " << entryDirName << " (cluster #" << clusterNum << ")" << std::endl;
        std::vector<FAT16DirEntry> subDirEntries;

        std::cout << "Start sector address of cluster #" << clusterNum << ": " << startSectorAddress << std::endl;

        if (clusterNum >= 0xFFF8) { // Якщо кластер знаходиться в діапазоні зарезервованих або пошкоджених.
            std::cerr << "Error: Found a bad or reserved cluster pointer number: " << clusterNum << std::endl;
            isDiskDataValid = false;
            continue;
        }

        if (!readDataCluster(fd, bytesPerSec, startSectorAddress, sectorsPerCluster, subDirEntries)) {
            std::cout << "Failed to read entries from the data region." << std::endl;
        } else {
            // Виводимо результати
            std::cout << "Found entries in data region:" << std::endl;
            for (const auto &subEntry: subDirEntries) {
                std::cout << "  Entry: " << subEntry.DIR_Name << std::endl;
            }
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
            AnalyzeDiskData16(fd, bytesPerSec, sectorsPerCluster, dataStartSector, subDirEntries);

        }
    }

    std::cout << "File and folder verification is complete." << std::endl;
    return isDiskDataValid;
}



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