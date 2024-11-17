#ifndef FATCHECKER_ANALYZERSFAT32_HPP
#define FATCHECKER_ANALYZERSFAT32_HPP

#include "FATstructs.hpp"
#include <vector>
#include <unordered_set>

// Функції для аналізу FAT таблиць
void AnalyzeMainFAT32(const uint8_t* fatBuffer, const std::vector<uint8_t*>& fatCopies, int fatSize, uint16_t bytesPerSec, bool fixErrors);
void AnalyzeCopyFAT32();
bool analyzeFAT32Tables(const std::vector<uint32_t*>& FATs, int FATSize, uint16_t bytesPerSec, bool fixErrors);

// Функції для аналізу директорій
bool AnalyzeRootDir32(FILE *file, uint32_t rootCluster, uint16_t bytesPerSec, uint8_t secPerClus, const uint32_t *FAT, uint32_t FATSize, std::vector<FAT32DirEntry>& rootDirEntries, bool fixErrors);
void AnalyzeDiskData32(FILE *file, uint16_t bytesPerSec, uint8_t secPerClus, uint32_t dataStartSector, const std::vector<FAT32DirEntry> &dirEntries, uint32_t *FAT, uint32_t FATSize, bool fixErrors);

// Функції для завантаження та перевірки даних
std::vector<uint32_t> loadFAT32Table(const uint8_t* fatBuffer, int fatSizeBytes);
bool readDataCluster32(FILE *file, uint16_t bytesPerSec, uint32_t startCluster, uint8_t secPerClus, uint32_t dataStartSector, std::vector<FAT32DirEntry> &entries);
bool readFAT32Tables(FILE *file, std::vector<uint32_t*>& FATs, int FATSize, int startSector, int numberOfFATs, uint16_t bytesPerSec);

// Перевірка інваріантів FAT32
bool isBootFAT32Invalid(extFAT32* bpb);
void checkLostClusters(const std::vector<uint32_t>& FAT, uint32_t FATSize, const std::unordered_set<uint32_t>& usedClusters, bool fixErrors);

// Функції для аналізу кластерів
void analyzeClusterUsage32(std::vector<uint32_t>& FAT, uint32_t FATSize, const std::vector<FAT32DirEntry>& directoryEntries, bool fixErrors);

// Функції для аналізу даних
bool AnalyzeDiskData32(FILE *file, uint32_t dataStartCluster, uint16_t bytesPerSec, uint8_t secPerClus, const uint32_t *FAT, uint32_t FATSize, std::vector<FAT32DirEntry>& dataDirEntries, std::vector<FileEntry>& fileEntries, bool fixErrors);

// Допоміжні функції
void printFAT32Table(const uint32_t* FAT, int FATSize, uint16_t bytesPerSec);

#endif //FATCHECKER_ANALYZERSFAT32_HPP
