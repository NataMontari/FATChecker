//#include <iostream>
//#include <fstream>
//#include <vector>
//#include <cstring>
//#include <cstdint>
//
//#pragma pack(push, 1)
//struct BootRecord {
//    uint8_t jumpInstruction[3]; // Стартовий код для завантаження
//    char oemID[8]; // OEM ID
//    uint16_t bytesPerSector; // Кількість байт на сектор
//    uint8_t sectorsPerCluster; // Кількість секторів на кластер
//    uint16_t reservedSectors; // Кількість зарезервованих секторів
//    uint8_t numFATs; // Кількість таблиць FAT
//    uint16_t rootDirEntries; // Кількість записів кореневого каталогу
//    uint16_t totalSectors; // Загальна кількість секторів
//    uint8_t mediaType; // Тип носія
//    uint16_t FATSize; // Розмір одного FAT
//    uint16_t sectorsPerTrack; // Кількість секторів на доріжці
//    uint16_t numHeads; // Кількість головок
//    uint32_t hiddenSectors; // Кількість прихованих секторів
//    uint32_t totalSectorsLarge; // Велика кількість секторів (для великих дисків)
//    uint32_t FATStart; // Стартовий сектор таблиці FAT
//    uint32_t dataStart; // Стартовий сектор для даних
//    uint32_t rootDirStart; // Стартовий сектор для кореневого каталогу
//    uint32_t dataClustersStart; // Початковий сектор для даних кластерів
//};
//#pragma pack(pop)
//
//class FATFileSystem{
//public:
//    FATFileSystem(const std::string& imagePath): imagePath(imagePath){
//        loadBootRecord();
//    }
//
//    bool loadBootRecord() {
//        std::ifstream file(imagePath, std::ios::binary);
//        if (!file.is_open()) {
//            std::cerr << "Failed to open image file." << std::endl;
//            return false;
//        }
//
//        // Зчитуємо Boot Record
//        file.seekg(0, std::ios::beg);
//        file.read(reinterpret_cast<char*>(&bootRecord), sizeof(bootRecord));
//        file.close();
//
//        return true;
//    }
//
//    virtual bool read_sector(uint32_t sector, uint8_t* buffer) {
//        std::ifstream device(imagePath, std::ios::binary);
//        if (!device) {
//            std::cerr << "Couldn't open device: " << imagePath << std::endl;
//            return false;
//        }
//
//        device.seekg(sector * sector_size, std::ios::beg);
//        device.read(reinterpret_cast<char*>(buffer), sector_size);
//
//        return device.good();
//    }
//private:
//    struct DirectoryEntry {
//        char filename[11]; // Ім'я файлу (для FAT12/16)
//        uint8_t attributes; // Атрибути файлу
//        uint16_t firstCluster; // Перший кластер
//        uint32_t fileSize; // Розмір файлу
//    };
//
//    BootRecord bootRecord;
//    std::vector<uint16_t> FATTable;
//    std::vector<DirectoryEntry> rootDirectory;
//    std::string imagePath;
//};