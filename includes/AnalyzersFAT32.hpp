#ifndef FATCHECKER_ANALYZERSFAT32_HPP
#define FATCHECKER_ANALYZERSFAT32_HPP

#include "FATstructs.hpp"
#include <vector>
#include <unordered_set>

// аналіз FAT таблиць

void AnalyzeCopyFAT32();
bool analyzeFAT32Tables(const std::vector<uint32_t*>& FATs, int FATSize, uint16_t bytesPerSec, int startFATSector, bool fixErrors);

// аналіз директорій
bool AnalyzeRootDir32(FILE *file, uint32_t rootCluster, uint32_t dataStartSector, uint16_t bytesPerSec, uint8_t secPerClus, const uint32_t *FAT, uint32_t FATSize, std::vector<FAT32DirEntry>& rootDirEntries, std::vector<FAT32DirEntry>& dataDirEntries, std::vector<FileEntry>& fileEntries, bool fixErrors);

// завантаження та перевірка даних
std::vector<uint32_t> loadFAT32Table(const uint8_t* fatBuffer, int fatSizeBytes);
bool readRootDirectoryFAT32(FILE *file, uint16_t bytesPerSec, uint8_t secPerClus, uint32_t dataStartSector,
                            uint32_t rootCluster, const uint32_t *FAT, uint32_t FATSize, std::vector<FAT32DirEntry> &entries) ;
bool readDataCluster32(FILE *file, uint16_t bytesPerSec, uint32_t startCluster, uint8_t secPerClus, uint32_t dataStartSector, std::vector<FAT32DirEntry> &entries);
bool readFAT32Tables(FILE *file, std::vector<uint32_t*>& FATs, int FATSize, int startSector, int numberOfFATs, uint16_t bytesPerSec);
bool check_date32(uint16_t date_value) ;
bool check_time32(uint16_t time_value);
bool checkEntry32(const FAT32DirEntry& entry);
bool isClusterValid32(uint32_t cluster, uint32_t maxCluster);
bool isValidName32(const std::string& name, int longNameFlag) ;
std::string extractLFN32(const LFNEntry& entry) ;
void printFileInfo32( FileEntry fileEntry, FAT32DirEntry entry);


// Перевірка інваріантів FAT32
bool isBootFAT32Invalid(extFAT32& bpb, bool fixErrors);
void checkLostClusters(const std::vector<uint32_t>& FAT, uint32_t FATSize, const std::unordered_set<uint32_t>& usedClusters, bool fixErrors);

//аналіз кластерів
void analyzeClusterUsage32(std::vector<uint32_t>& FAT, uint32_t FATSize, const std::vector<FAT32DirEntry>& directoryEntries, bool fixErrors);

// аналіз даних
void AnalyzeDiskData32(FILE *file, uint16_t bytesPerSec, uint8_t secPerClus, uint32_t dataStartSector, const std::vector<FAT32DirEntry> &dirEntries, std::vector<FileEntry> &fileEntries, uint32_t *FAT, uint32_t FATSize, bool fixErrors, bool isRootDir = true);
void handleInvalidBootSector32(extFAT32& bpb) ;
bool attemptRestoreFromBackup32(extFAT32& bpb);
bool restoreFromBackup32(extFAT32& bpb);

// допоміжні функції
void printFAT32Table(const uint32_t* FAT, int FATSize, uint16_t bytesPerSec);
struct LFNEntry32 {
    uint8_t LDIR_Ord;          // Порядок фрагмента
    uint16_t LDIR_Name1[5];    // Перша частина імені (5 символів)
    uint8_t LDIR_Attr;         // Атрибут (завжди 0x0F для LFN)
    uint8_t LDIR_Type;         // Тип (зазвичай 0x00)
    uint8_t LDIR_Chksum;       // Контрольна сума
    uint16_t LDIR_Name2[6];    // Друга частина імені (6 символів)
    uint16_t LDIR_FstClusLO;   // Завжди 0 для LFN
    uint16_t LDIR_Name3[2];    // Третя частина імені (2 символи)
};
#endif //FATCHECKER_ANALYZERSFAT32_HPP