#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <fcntl.h>
#include "file.hpp"
// #include <io.h>// для вінди (наступний закоментуєш а цей навпаки)
#include <unistd.h> // для лінукса
#include <iomanip> // Для std::hex
#include <vector>
#include <algorithm>

#include "FATstructs.hpp"
#include "AnalyzersFAT16.hpp"
#include "AnalyzersFAT32.hpp"
#include "AnalyzersFAT12.hpp"

// #define DEBUG_PRNT

// Читання кореневої директорії
bool readRootDirectory(FILE *file, uint16_t bytesPerSec, uint16_t resvdSecCnt, uint8_t numFATs, uint16_t FATSize, uint16_t rootEntCnt, std::vector<FAT16DirEntry>& rootDirEntries) {
    // Обчислюємо початковий сектор кореневої директорії
    uint32_t firstRootDirSecNum = resvdSecCnt + (numFATs * FATSize);

    // Обчислюємо кількість секторів, яку займає коренева директорія
    uint32_t rootDirSectors = ((rootEntCnt * 32) + (bytesPerSec - 1)) / bytesPerSec;

    // Обчислюємо розмір кореневої директорії у байтах
    size_t rootDirSize = rootDirSectors * bytesPerSec;

    // Виділяємо пам'ять для зчитування кореневої директорії
    uint8_t* rootDirData = new uint8_t[rootDirSize];

    // Позиціонуємося на початок кореневої директорії
    if (fseek(file, firstRootDirSecNum * bytesPerSec, SEEK_SET) != 0) {
        perror("Failed to seek to the root directory");
        delete[] rootDirData;
        return false;
    }


    // Читаємо кореневу директорію
    if (fread(rootDirData, 1, rootDirSize, file) != rootDirSize) {
        perror("Failed to read root directory");
        delete[] rootDirData;
        return false;
    }

    // Обробляємо кожен запис директорії
    size_t numEntries = rootDirSize / sizeof(FAT16DirEntry);
    for (size_t i = 0; i < numEntries; i++) {
        FAT16DirEntry entry;
        std::memcpy(&entry, rootDirData + i * sizeof(FAT16DirEntry), sizeof(FAT16DirEntry));

        // Додаємо вектор зчитаних записів
        rootDirEntries.push_back(entry);
    }

    delete[] rootDirData;
    return true;
}


// Підрахунок та виведення результатів у директорії
void countDirectoryEntries(const std::vector<FAT16DirEntry>& entries) {
    int volumeIDCount = 0;
    int directoryCount = 0;
    int fileCount = 0;

    for (const auto& entry : entries) {
        // Перевірка атрибутів
        if (entry.DIR_Attr == 15) { // Пропустити записи з довгим ім'ям
            continue;
        }

        if (entry.ATTR_VOLUME_ID) { // Volume_ID
            volumeIDCount++;
        } else if (entry.ATTR_DIRECTORY) { // Directory
            directoryCount++;
        } else if (entry.ATTR_ARCHIVE) { // File (Archive)
            fileCount++;
        }
    }

    // Виведення результатів
    std::cout << "Num of root directories (Volume_ID): " << volumeIDCount << std::endl;
    std::cout << "Num of directories: " << directoryCount << std::endl;
    std::cout << "Num of files: " << fileCount << std::endl;
}


// Виведення записів кореневої директорії
void printRootDirectoryEntries(const std::vector<FAT16DirEntry>& rootDirEntries) {
    int entryCount = 1;
    for (const auto& entry : rootDirEntries) {
        // Виводимо ім'я файлу (перетворюємо на строку)
        std::string fileName(entry.DIR_Name, 11);
        fileName.erase(std::remove(fileName.begin(), fileName.end(), ' '), fileName.end()); // Видаляємо пробіли

        std::cout << "File entry " << entryCount++ << ":\n";
        std::cout << "DIR_Name:       " << fileName << "\n";
        std::cout << "DIR_Attr:       " << static_cast<int>(entry.DIR_Attr)<< "\n";
        std::cout << "DIR_NTRes:      " << static_cast<int>(entry.DIR_NTRes) << "\n";
        std::cout << "DIR_CrtTimeTenth: " << static_cast<int>(entry.DIR_CrtTimeTenth) << "\n";
        std::cout << "DIR_CrtTime:    " << entry.DIR_CrtTime << "\n";
        std::cout << "DIR_CrtDate:    " << entry.DIR_CrtDate << "\n";
        std::cout << "DIR_LstAccDate: " << entry.DIR_LstAccDate << "\n";
        std::cout << "DIR_WrtTime:    " << entry.DIR_WrtTime << "\n";
        std::cout << "DIR_WrtDate:    " << entry.DIR_WrtDate << "\n";

        std::cout << "DIR_FstClusLO:  " << entry.DIR_FstClusLO<< "\n";
        std::cout << "DIR_FileSize:   " << entry.DIR_FileSize << "\n";
        std::cout << std::string(30, '-') << "\n"; // Розділювач між записами
    }
}


// Зчитування FAT таблиць
bool readFAT16Tables(FILE *file, std::vector<uint16_t*>& FATs, int FATSize, int startSector, int numberOfFATs, uint16_t bytesPerSec) {
#ifdef DEBUG_PRNT
    std::cout<< "---------------------------------"<<std::endl;
    std::cout<<"Trying to read FAT tables\n"<<std::endl;
#endif

    const int startFATPosition = startSector * bytesPerSec;
//    std::cout<<"FAT "
    for (int i = 0; i < numberOfFATs; i++) {
        // Обчислюємо розмір масиву для зчитування FAT таблиці
        uint16_t *FAT = new uint16_t[(FATSize * bytesPerSec) / sizeof(uint16_t)];
#ifdef DEBUG_PRNT
        std::cout << "Current position: "<< startFATPosition + FATSize*i*bytesPerSec << std::endl;
#endif
        // Зчитування FAT таблиці
        if (fseek(file, startFATPosition + FATSize*i*bytesPerSec, SEEK_SET) == -1 ||
            fread( FAT, sizeof(uint16_t), FATSize * bytesPerSec / sizeof(uint16_t), file) != FATSize * bytesPerSec  / sizeof(uint16_t)) {
            perror("Failed to read FAT table");
            delete[] FAT; // Звільняємо пам'ять
            return false;
        }
#ifdef DEBUG_PRNT
        std::cout << "Successfully read FAT table " << i + 1 << " from sector " << (startSector + i) << std::endl;
#endif
        FATs.push_back(FAT); // Додаємо зчитану таблицю до вектору
    }
    return true;
}


// Визначення типу файлової системи
int determineFATType(extFAT12_16 *bpb) {
    std::string fileSysType(bpb->BS_FilSysType, 3); // копіювання перших трьох символів у стрічку

    // Невідомий тип FAT
    if (fileSysType != "FAT"){
        auto *bpb32 = (extFAT32 *)bpb; // Перевірка на FAT32
        std::string fileSysType32(bpb32->BS_FilSysType,5); // копіювання перших 5и символів у стрічку
        if (fileSysType32 == "FAT32"){
            return 32;
        }
        return -1;
    }

    // Розмір FAT
    uint32_t fatSize = (bpb->basic.BPB_FATSz16 != 0) ? bpb->basic.BPB_FATSz16 : 0;


    // Загальна к-сть секторів
    uint32_t totalSectors = (bpb->basic.BPB_TotSec16 != 0) ? bpb->basic.BPB_TotSec16 : bpb->basic.BPB_TotSec32;


    // К-сть секторів для даних
    uint32_t rootDirSectors = ((bpb->basic.BPB_RootEntCnt * 32) + (bpb->basic.BPB_BytsPerSec - 1)) / bpb->basic.BPB_BytsPerSec;
//    std::cout<<" determine FAT"<<std::endl;
    uint32_t dataSectors = totalSectors - (bpb->basic.BPB_RsvdSecCnt + (bpb->basic.BPB_NumFATs * fatSize) + rootDirSectors);

    // К-сть кластерів у файловій системі
    uint32_t countOfClusters = dataSectors / bpb->basic.BPB_SecPerClus;
//    std::cout<<" determine FAT"<<std::endl;

    // Визначаємо тип FAT
    if (countOfClusters < 4085) {
        return 12;  // FAT12
    } else if (countOfClusters < 65525) {
        return 16;  // FAT16
    }
    return -1;
}

// Функція для виводу значень структури BasicFAT
void printBasicFAT(extFAT12_16 &bpb) {
    std::cout<< "---------------------------------"<<std::endl;
    std::cout << "Boot Sector Information:\n";
    std::cout << "BS_jmpBoot: ";
    for (int i = 0; i < 3; ++i) {
        std::cout << (int)bpb.basic.BS_jmpBoot[i] << " ";
    }
    std::cout << "\nBS_OEMName: " << bpb.basic.BS_OEMName <<"\n";
    std::cout << "BPB_BytsPerSec: " << bpb.basic.BPB_BytsPerSec <<"\n";
    std::cout << "BPB_SecPerClus: " << (int)bpb.basic.BPB_SecPerClus <<"\n";
    std::cout << "BPB_RsvdSecCnt: " << bpb.basic.BPB_RsvdSecCnt<<"\n";
    std::cout << "BPB_NumFATs: " << (int)bpb.basic.BPB_NumFATs <<"\n";
    std::cout << "BPB_RootEntCnt: " << bpb.basic.BPB_RootEntCnt <<"\n";
    std::cout << "BPB_TotSec16: " << bpb.basic.BPB_TotSec16<<"\n";
    std::cout << "BPB_Media: " << (int)bpb.basic.BPB_Media<< "\n";
    std::cout << "BPB_FATSz16: " << bpb.basic.BPB_FATSz16<<"\n";
    std::cout << "BPB_SecPerTrk: " << bpb.basic.BPB_SecPerTrk << "\n";
    std::cout << "BPB_NumHeads: " << bpb.basic.BPB_NumHeads << "\n";
    std::cout << "BPB_HiddSec: " << bpb.basic.BPB_HiddSec <<  "\n";
    std::cout << "BPB_TotSec32: " << bpb.basic.BPB_TotSec32 <<"\n";

    std::string fileSysType(bpb.BS_FilSysType, 3);
    std::cout << "BS_FilSysType: " << fileSysType <<"\n";

}

bool readBootSector(uint8_t *bootSector, int bootSector_size) {
#ifdef DEBUG_PRNT
    std::cout << "Size of boot Sector: " << bootSector_size << std::endl;
    std::cout << "---------------------------------" << std::endl;
    std::cout << "Trying to read boot sector" << std::endl;
#endif

    // Переміщення в початок файлу
    if (fseek(fp, 0, SEEK_SET) != 0) {
        perror("Failed to seek boot sector");
        return false;
    }

    // Зчитування завантажувального сектора
    size_t bytesRead = fread(bootSector, 1, bootSector_size, fp);
    if (bytesRead != bootSector_size) {
        perror("Failed to read boot sector");
        return false;
    }

#ifdef DEBUG_PRNT
    std::cout << "Successfully read boot sector" << std::endl;
    std::cout << "Boot Sector Hex Dump:" << std::endl;
    for (int i = 0; i < bootSector_size; i++) {
        std::cout << std::hex << (int)bootSector[i] << " ";
        if ((i + 1) % 16 == 0) std::cout << std::endl; // Вивід через кожні 16 байт
    }
    std::cout << std::dec;
#endif

    return true;
}

bool writeBootSector(FILE *file, uint8_t *bootSector, size_t bootSector_size) {

    // Зміщення у початок файлу
    if (fseek(file, 0, SEEK_SET) == -1) {
        perror("Failed to seek to the beginning of the file");
        return false;
    }

    // Запис даних у завантажувальний сектор
    ssize_t bytes_written = fwrite(bootSector, 1, bootSector_size, file);
    if (bytes_written != bootSector_size) {
        perror("Failed to write boot sector");
        return false;
    }

    return true; // Успішний запис
}


void printFATTable(const uint16_t *FAT, int FATSize, uint16_t bytesPerSec) {
    std::cout << "FAT Table Contents (Total Bytes: " << FATSize * bytesPerSec << "):" << std::endl;

    // Виведення значень FAT таблиці
    size_t totalBytes = FATSize*2;
    for (size_t i = 0; i < totalBytes; ++i) {
        // Виведення значень у шістнадцятковому форматі (записи у FAT)
        std::cout << "Byte " << std::setw(5) << i << ": 0x"
                  << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(FAT[i]) << std::dec << std::endl;
        // Виведення по 16 біт
        if ((i + 1) % 16 == 0) {
            std::cout << std::endl; // Пустий рядок для відділення
        }
    }
}

void fs_open(const char *path, int rw) {
    // Встановлення режиму відкриття
    const char *mode = rw ? "rb+" : "rb";  // "rb+" для читання і запису, "rb" для читання

    // Відкриття диска
    fp = fopen(path, mode);
    if (fp == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
}

void printUsage() {
    std::cout << "Usage: ./FATChecker.exe <path_to_disk>: [options]\n";
    std::cout << "Options:\n";
    std::cout << "  /F          Attempt to fix errors\n";
}

int main(int argc, char* argv[]) {
    BasicFAT  myBasicFAT;
//    std::cout<<sizeof(myBasicFAT)<<std::endl;
//    std::cout<<sizeof(BasicFAT)<<std::endl;
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string imagePath = argv[1];
    bool fixErrors = false;
    bool isImageFile = false; // Додатковий параметр для перевірки, чи працюємо з файлом-образом

    // Перевірка на наявність опції --fix
    if (argc > 2 && std::string(argv[2])== "/F") {
        fixErrors = true;
    }else if(argc > 2 && std::string (argv[2]) == "--image"){
        isImageFile = true;
    }
    // Перевірка, чи є аргумент для образу файлу
    if (argc > 3 && std::string(argv[3]) == "/IM") {
        isImageFile = true;
    }else if (argc > 3 && std::string(argv[3]) == "/F"){
        fixErrors = true;
    }

#ifdef DEBUG_PRNT
    std::cout << "Trying to open file" << std::endl << std::flush;
    std::cout << "Fix errors: "<<fixErrors<<std::endl;
#endif
    std::string drLetter = argv[1]; // GET THE RIGHT DEVICE
    std::string device = (drLetter);
    std::cout<<"Analyzing disk/image: "<<device<<std::endl;

    // Використання функції для відкриття фізичного диска
    fs_open(device.c_str(), 1); // rw = 1 для читання та запису

#ifdef DEBUG_PRNT
    std::cout<<"File was opened"<<std::endl;
#endif
    uint8_t bootSector[512]; // Масив із 512 байтів
    int fatType;
//    std::cout<<"Size of BasicFAT: "<< sizeof(myBasicFAT)<<std::endl;


    if(!readBootSector(bootSector, 512)){
        fclose(fp);
        std::cout<<"Can't read boot sector. Program was terminated. "<<std::endl;
        exit(EXIT_FAILURE);
    }


//    std::cout<<"Size of BasicFAT: "<< sizeof(myBasicFAT)<<std::endl;
#ifdef DEBUG_PRNT
    std::cout<<" Trying to construct a struct"<<std::endl;
#endif
    extFAT12_16 bpb;
    memcpy(&bpb, bootSector, sizeof(extFAT12_16));

#ifdef DEBUG_PRNT
    std::cout<<"successfully made a struct, determine fat type\n"<<std::endl;
#endif

    int fatVal;
    // вектор для зберігання FAT таблиць

    bool totalResult = true;
    switch(determineFATType(&bpb)){
        case 12: {
                printBasicFAT(bpb);
            std::cout<<"The type of the file system is FAT12\n"<<std::endl;
            //Перевірка справності boot сектора
            if (isBootFAT12Invalid(&bpb, fixErrors)) {
                std::cout << "Boot sector is corrupted. Program was terminated." << std::endl;
                exit(EXIT_FAILURE);
            }
            //Підвантаження FAT таблиць та їх перевірка
            AnalyzeCopyFAT16();
//            AnalyzeRootDir16();
//            AnalyzeDiskData16();

            break;
        }
        case 16: {
                printBasicFAT(bpb);
                std::cout<<"---------------------------------------------------------"<<std::endl;

                std::cout << "The type of the file system is FAT16\n" << std::endl;

            std::vector<uint16_t*> FATs; // Вектор для зберігання FAT таблиць

                std::cout<<"---------------------------------------------------------"<<std::endl;
            //Перевірка справності boot сектора
            if (isBootFAT16Invalid(bpb, fixErrors)) {
                std::cout << "Boot sector is corrupted. Program was terminated." << std::endl;
                exit(EXIT_FAILURE);
            }
                const int FATSize = bpb.basic.BPB_FATSz16; // Розмір FAT таблиці
                const uint16_t bytesPerSec = bpb.basic.BPB_BytsPerSec;
                const int startFATSector = bpb.basic.BPB_RsvdSecCnt;
                const int numberOfFATs = bpb.basic.BPB_NumFATs; // Кількість FAT таблиць
                std::cout<<"No errors found in the critical parts of the boot sector. Proceeding with reading the FAT tables."<<std::endl;


            //Підвантаження FAT таблиць та їх перевірка
            if (!readFAT16Tables(fp, FATs, FATSize, startFATSector, numberOfFATs, bytesPerSec)) {
                fclose(fp);
                std::cout<<"Unable to read FAT tables. FAT tables region is corrupted, please check your disk."<<std::endl;
                exit(EXIT_FAILURE);
            }
//
//            fatVal = FATs[1][0];
//            std::cout << fatVal <<std::endl;
//            FATs[1][0] = 10;

            // Аналіз FAT таблиць
                std::cout<<"---------------------------------------------------------"<<std::endl;


            totalResult = totalResult && analyzeFAT16Tables(FATs, FATSize, bytesPerSec, startFATSector, fixErrors);

#ifdef DEBUG_PRNT
            printFATTable(FATs[0], FATSize, bytesPerSec);
#endif

            // Звільнення пам'яті
//            for (auto FAT : FATs) {
//                delete[] FAT;
//            }
//            AnalyzeMainFAT16(bpb);
//            AnalyzeCopyFAT16();
            std::vector<FAT16DirEntry> rootDirEntries;
            int rootEntCnt = bpb.basic.BPB_RootEntCnt;
            int resvdSecCnt = bpb.basic.BPB_RsvdSecCnt;
            int sectorsPerClus = bpb.basic.BPB_SecPerClus;
            int rootStartSector = resvdSecCnt + numberOfFATs*FATSize;
            int dataRegionStartSector = rootStartSector + rootEntCnt*32/bytesPerSec;

            if (readRootDirectory(fp, bytesPerSec, resvdSecCnt, numberOfFATs, FATSize, rootEntCnt, rootDirEntries)) {
#ifdef DEBUG_PRNT
                std::cout << "Successfully read root directory." << std::endl;
#endif
                // Виведення записів кореневої директорії
            }
            // Виводимо вміст root directory
#ifdef DEBUG_PRNT
            // printRootDirectoryEntries(rootDirEntries);
#endif
            // Виводимо к-сть root directories, директорій та файлів
            std::vector<FAT16DirEntry> dataDirEntries;
            std::vector<FileEntry> fileEntries;
            bool isRootValid = AnalyzeRootDir16(rootDirEntries, dataDirEntries, fileEntries, fixErrors);

            totalResult = totalResult && isRootValid;
            if (!isRootValid){
                std::cout<<"Warning! Found problems in the root directory."<<std::endl;
            }
#ifdef DEBUG_PRNT
            for (const auto& entry: dataDirEntries){
                for(const auto& letter: entry.DIR_Name){
                    std::cout<<letter;
                }
                std::cout<<std::endl;
            }
#endif
                std::cout<<"Analyzing file and directory region"<<std::endl;
            bool isDataValid = AnalyzeDiskData16(fp, bytesPerSec, sectorsPerClus, dataRegionStartSector, dataDirEntries, fileEntries, fixErrors);

                std::cout<<"---------------------------------------------------------"<<std::endl;
            analyzeClusterInvariants(FATs[0], FATSize*2, bytesPerSec, sectorsPerClus, fileEntries, fixErrors);
            for (auto &FAT: FATs){
                delete[] FAT;
            }
                std::cout<<"---------------------------------------------------------"<<std::endl;
//            loadFAT16Table(FATs[0], FATSize);

//            PrintFileEntries(fileEntries);
//            std::cout<<"check: "<<FATs[0][0]<<std::endl;
            break;
        }
        case 32: {
                printBasicFAT(bpb);
                extFAT32 *bpb32 = reinterpret_cast<extFAT32 *>(&bpb);
                std::cout<<"BPB_FATSz32:"<<bpb32->BPB_FATSz32<<std::endl;
                std::cout<<"---------------------------------------------------------"<<std::endl;
            std::cout << "The type of the file system is FAT32\n" << std::endl;

            // посилання на структуру FAT32


            //перевірка завантажувального сектора
            if (isBootFAT32Invalid(bpb32, fixErrors)) {
                std::cerr << "Boot sector is invalid. Terminating program.\n";
                fclose(fp);
                exit(EXIT_FAILURE);
            }
                std::cout<<"---------------------------------------------------------"<<std::endl;
            const uint16_t bytesPerSec = bpb32->basic.BPB_BytsPerSec;
            const uint8_t secPerClus = bpb32->basic.BPB_SecPerClus;
            const uint16_t rsvdSecCnt = bpb32->basic.BPB_RsvdSecCnt;
            const uint8_t numFATs = bpb32->basic.BPB_NumFATs;
            const uint32_t fatSize32 = bpb32->BPB_FATSz32;
            const uint32_t rootCluster = bpb32->BPB_RootClus;
            const uint32_t dataStartSector = rsvdSecCnt + (numFATs * fatSize32);
                int FATSize=  fatSize32*bytesPerSec/sizeof(uint32_t);

            std::vector<uint32_t*> FATs;

            // підтягування FAT таблиць (+аналіз далі)
            if (!readFAT32Tables(fp, FATs, fatSize32, rsvdSecCnt, numFATs, bytesPerSec)) {
                fclose(fp);
                exit(EXIT_FAILURE);
            }
            if (!analyzeFAT32Tables(FATs, fatSize32, bytesPerSec, fixErrors)) {
                std::cerr << "FAT32 table analysis failed.\n";
                fclose(fp);
                for (auto FAT : FATs) delete[] FAT;
                exit(EXIT_FAILURE);
            }
                std::cout<<"---------------------------------------------------------"<<std::endl;

            // аналіз кореневої директорії

                std::vector<FAT32DirEntry> rootDirEntries;
                std::vector<FAT32DirEntry> dataDirEntries;
                std::vector<FileEntry> fileEntries;
            if (!AnalyzeRootDir32(fp, rootCluster, dataStartSector, bytesPerSec, secPerClus, FATs[0], fatSize32, rootDirEntries, dataDirEntries, fileEntries, fixErrors)) {
                std::cerr << "Root directory analysis failed.\n";
                fclose(fp);
                for (auto FAT : FATs) delete[] FAT;
                exit(EXIT_FAILURE);
            }
                std::cout<<"---------------------------------------------------------"<<std::endl;
                for(auto dir: dataDirEntries)
                {
                    std::cout<<dir.DIR_Name<<std::endl;

                };
                std::cout << "=== Analyzing Disk Data (Clusters) ===" << std::endl;
            // аналіз даних (кластерів)
            AnalyzeDiskData32(fp, bytesPerSec, secPerClus, dataStartSector, dataDirEntries, fileEntries, FATs[0], FATSize, fatSize32, fixErrors);
                std::cout<<"---------------------------------------------------------"<<std::endl;
            // аналіз використання кластерів
            std::vector<uint32_t> tempFAT(FATs[0], FATs[0] + fatSize32);
            analyzeClusterUsage32(tempFAT, fatSize32, rootDirEntries, fixErrors);

            // перевірка загублених кластерів
            std::unordered_set<uint32_t> usedClusters;
            for (const auto &entry : rootDirEntries) {
                uint32_t firstCluster = (entry.DIR_FstClusHI << 16) | entry.DIR_FstClusLO;
                if (firstCluster >= 2) {
                    usedClusters.insert(firstCluster);
                }
            }
            checkLostClusters(std::vector<uint32_t>(FATs[0], FATs[0] + fatSize32), fatSize32, usedClusters, fixErrors);

            // звільнення пам'яті для FAT таблиць
            for (auto FAT : FATs) delete[] FAT;

            break;
        }
        case 33:break; //exFAT
        case 34:break; //vFAT
        default:
            std::cout<<"The type of the file system isn't FAT"<<std::endl;
            exit(EXIT_FAILURE);
    }

    if (totalResult){
        std::cout<<"The program has scanned the file system and found no major problems."<<std::endl;
        std::cout<<"No further action is required."<<std::endl;
        // TO DO add statistic
    }
    else{
        std::cout<<"The program found the following errors: "<<std::endl;
    }





    // Закриття дескриптора
    fclose(fp);

//    // Заглушка для аналізу файлової системи
//    std::cout << "Analyzing file system image: " << imagePath << "\n";
//    std::cout << "Fix errors: " << (fixErrors ? "Yes" : "No") << "\n";

    // Тут будуть виклики функцій аналізу та виправлення FAT

    return 0;
}