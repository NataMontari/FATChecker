#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#include <iomanip> // Для std::hex
#include <cstdint>
#include "FATstructs.hpp"
#include "AnalyzersFAT16.hpp"
#include "AnalyzersFAT32.hpp"

int fd;



// Функція для визначення типу файлової системи
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

    // Визначаємо розмір FAT
    uint32_t fatSize = (bpb->basic.BPB_FATSz16 != 0) ? bpb->basic.BPB_FATSz16 : 0;


    // Загальна кількість секторів
    uint32_t totalSectors = (bpb->basic.BPB_TotSec16 != 0) ? bpb->basic.BPB_TotSec16 : bpb->basic.BPB_TotSec32;


    // Кількість секторів для даних
    uint32_t rootDirSectors = ((bpb->basic.BPB_RootEntCnt * 32) + (bpb->basic.BPB_BytsPerSec - 1)) / bpb->basic.BPB_BytsPerSec;
//    std::cout<<" determine FAT"<<std::endl;
    uint32_t dataSectors = totalSectors - (bpb->basic.BPB_RsvdSecCnt + (bpb->basic.BPB_NumFATs * fatSize) + rootDirSectors);

    // Кількість кластерів у файловій системі
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
void printBasicFAT(extFAT12_16 *bpb) {
    std::cout << "Boot Sector Information:\n";
    std::cout << "BS_jmpBoot: ";
    for (int i = 0; i < 3; ++i) {
        std::cout << (int)bpb->basic.BS_jmpBoot[i] << " ";
    }
    std::cout<<sizeof(bpb->basic.BS_jmpBoot);
    std::cout << "\nBS_OEMName: " << bpb->basic.BS_OEMName << " "<< sizeof(bpb->basic.BS_OEMName)<<"\n";
    std::cout << "BPB_BytsPerSec: " << bpb->basic.BPB_BytsPerSec << " "<<sizeof(bpb->basic.BPB_BytsPerSec)<<"\n";
    std::cout << "BPB_SecPerClus: " << (int)bpb->basic.BPB_SecPerClus << " "<<sizeof(bpb->basic.BPB_SecPerClus)<<"\n";
    std::cout << "BPB_RsvdSecCnt: " << bpb->basic.BPB_RsvdSecCnt << " "<<sizeof(bpb->basic.BPB_RsvdSecCnt)<<"\n";
    std::cout << "BPB_NumFATs: " << (int)bpb->basic.BPB_NumFATs <<  " "<<sizeof(bpb->basic.BPB_NumFATs)<<"\n";
    std::cout << "BPB_RootEntCnt: " << bpb->basic.BPB_RootEntCnt << " "<<sizeof(bpb->basic.BPB_RootEntCnt)<<"\n";
    std::cout << "BPB_TotSec16: " << bpb->basic.BPB_TotSec16 << " "<<sizeof(bpb->basic.BPB_TotSec16)<<"\n";
    std::cout << "BPB_Media: " << (int)bpb->basic.BPB_Media <<" "<<sizeof(bpb->basic.BPB_Media)<< "\n";
    std::cout << "BPB_FATSz16: " << bpb->basic.BPB_FATSz16 << " "<<sizeof(bpb->basic.BPB_FATSz16)<<"\n";
    std::cout << "BPB_SecPerTrk: " << bpb->basic.BPB_SecPerTrk << " "<<sizeof(bpb->basic.BPB_SecPerTrk)<< "\n";
    std::cout << "BPB_NumHeads: " << bpb->basic.BPB_NumHeads << " "<<sizeof(bpb->basic.BPB_NumHeads)<< "\n";
    std::cout << "BPB_HiddSec: " << bpb->basic.BPB_HiddSec << " "<<sizeof(bpb->basic.BPB_HiddSec)<<  "\n";
    std::cout << "BPB_TotSec32: " << bpb->basic.BPB_TotSec32 << " "<<sizeof(bpb->basic.BPB_TotSec32)<<"\n";

    std::string fileSysType(bpb->BS_FilSysType, 3);
    std::cout << "BS_FilSysType: " << fileSysType << " "<<sizeof(bpb->BS_FilSysType)<<"\n";

}

bool readBootSector(uint8_t *bootSector, int bootSector_size) {
//    uint8_t bootSector[512]; // Буфер для завантажувального сектора
    std::cout<<"Size of boot Sector: "<<bootSector_size<<std::endl;


    std::cout<<"trying to read boot sector"<<std::endl;
    // Зчитування завантажувального сектора
    if (lseek(fd, 0, SEEK_SET) == -1 || read(fd, bootSector, bootSector_size) != bootSector_size) {
        perror("Failed to read boot sector");
        return false;
    }
    std::cout<<"Successfully read boot sector"<<std::endl;

    std::cout << "Boot Sector Hex Dump:" << std::endl;
    for (int i = 0; i < 512; i++) {
        std::cout  <<bootSector[i] ;
        if ((i + 1) % 16 == 0) std::cout << std::endl; // Вивід через кожні 16 байт
    }
    std::cout<<std::dec;


    return true;
}

bool writeBootSector(int fd, uint8_t *bootSector, size_t bootSector_size) {
    // Зміщення у початок файлу
    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("Failed to seek to the beginning of the file");
        return false;
    }

    // Запис даних у завантажувальний сектор
    ssize_t bytes_written = write(fd, bootSector, bootSector_size);
    if (bytes_written != bootSector_size) {
        perror("Failed to write boot sector");
        return false;
    }

    return true; // Успішний запис
}



void fs_open(const char *path, int rw) {
    // Встановлення режиму відкриття
    int mode = rw ? O_RDWR | O_BINARY : O_RDONLY | O_BINARY;

    // Відкриття диска
    fd = open(path, mode);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Ваш код для роботи з fd...
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

    // Перевірка на наявність опції --fix
    if (argc > 2 && std::string(argv[2]) == "/F") {
        fixErrors = true;
    }


    std::cout << "Trying to open file" << std::endl << std::flush;
    std::cout << "Fix errors: "<<fixErrors<<std::endl;
    std::string drLetter = argv[1]; // GET THE RIGHT DEVICE
    std::string device = ("\\\\.\\" + drLetter + ":");
    std::cout<<device<<std::endl;
//    std::string drLetter = "D"; // GET THE RIGHT DEVICE
//    std::string device = ("\\\\.\\" + drLetter + ":");

//    std::FILE * fp = fopen(device.c_str(), "rb"); // read, binary
//
//    if(!fp) {
//        std::cerr << "Device '" + drLetter + "' could not be opened!" << std::endl;
//        exit (101);
//    }
//    else
//    {
//        std::cout<<"Successfully opened file"<<std::endl;
//    }

    // Використання функції для відкриття фізичного диска
    fs_open(device.c_str(), 1); // rw = 1 для читання та запису

    std::cout<<"File was opened"<<std::endl;
    uint8_t bootSector[512]; // Масив із 512 байтів
    int fatType;
    std::cout<<"Size of BasicFAT: "<< sizeof(myBasicFAT)<<std::endl;


    if(!readBootSector(bootSector, 512)){
        close(fd);
        std::cout<<"Can't read boot sector. Program was terminated. "<<std::endl;
        exit(EXIT_FAILURE);
    }


    std::cout<<"Size of BasicFAT: "<< sizeof(myBasicFAT)<<std::endl;
    std::cout<<" Trying to construct a struct"<<std::endl;
    extFAT12_16 *bpb = (extFAT12_16 *)bootSector;
    std::cout<<"successfully made a struct, determine fat type"<<std::endl;


    switch(determineFATType(bpb)){
        case 12: std::cout<<"File System FAT12\n"<<std::endl; break;
        case 16: {

            std::cout << "File System FAT16\n" << std::endl;

//            printBasicFAT(bpb);

            //Перевірка справності boot сектора
            if (isBootFAT16Invalid(bpb)) {
                std::cout << "Boot sector is corrupted. Program was terminated." << std::endl;
                exit(EXIT_FAILURE);
            }
            //Підвантаження FAT таблиць та їх перевірка
            AnalyzeMainFAT16();
            AnalyzeCopyFAT16();
            AnalyzeRootDir16();
            AnalyzeDiskData16();

            break;
        }
        case 32: {
            std::cout << "File System FAT32\n" << std::endl;
            //Перевірка справності boot сектора
            auto *bpb32 = (extFAT32 *) bootSector;
            if (isBootFAT32Invalid(bpb32)) {
                std::cout << "Boot sector is corrupted. Program was terminated." << std::endl;
                exit(EXIT_FAILURE);
            }
            //Підвантаження FAT таблиць та їх перевірка
            AnalyzeMainFAT32();
            AnalyzeCopyFAT32();
            AnalyzeRootDir32();
            AnalyzeDiskData32();

            break;
        }
        case 33:break; //exFAT
        case 34:break; //vFAT
        default:
            std::cout<<"Unknown file system type"<<std::endl;
            exit(EXIT_FAILURE);
    }

//        const char *newOEMName = "NataDOS5"; // Нова назва OEM (має бути 8 символів)
//        strncpy(bpb->BS_OEMName, newOEMName, sizeof(bpb->BS_OEMName));

    printBasicFAT(bpb);


//    std::string fileSystemType = getFileSystemType();
//    std::cout << "File system type: " << fileSystemType << std::endl;


//        ssize_t bytes_written = write(fd, &bootSector[0], sizeof(myBasicFAT));
//        std::cout<<"Size of BasicFAT: "<< sizeof(myBasicFAT)<<std::endl;
//        if (bytes_written != 512) {
//            std::cout << "Couldn't write to boot sector: " << strerror(errno) << std::endl;
//        }
//        if (!writeBootSector(fd, bootSector, sizeof(bootSector))) {
//            std::cerr << "Error writing boot sector" << std::endl;
//            close(fd);
//            return 1;
//        }
    // Закриття дескриптора
    close(fd);

//    // Заглушка для аналізу файлової системи
//    std::cout << "Analyzing file system image: " << imagePath << "\n";
//    std::cout << "Fix errors: " << (fixErrors ? "Yes" : "No") << "\n";

    // Тут будуть виклики функцій аналізу та виправлення FAT

    return 0;
}
