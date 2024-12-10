

#ifndef FATCHECKER_ANALYZERSFAT12_HPP
#define FATCHECKER_ANALYZERSFAT12_HPP

#include <iomanip>
void AnalyzeMainFAT12();
void AnalyzeCopyFAT12();
bool AnalyzeRootDir12(std::vector<FAT12DirEntry>& rootDirEntries, std::vector<FAT12DirEntry>& dataDirEntries,  std::vector<FileEntry>& fileEntries, bool fixErrors);
void AnalyzeDiskData12();
bool fixRootDirErrors12();
bool isBootFAT12Invalid(extFAT12_16* bpb, bool fixErrors);
bool analyzeFAT12Tables(const std::vector<uint8_t*>& FATs, int FATSize, uint16_t bytesPerSec, int startFATSector, bool fixErrors);
void writeFAT12Entry(uint8_t* FAT, int entryIndex, uint16_t value);
uint16_t readFAT12Entry(uint8_t* FAT, int entryIndex);
void handleInvalidBootSector12(extFAT12_16* bpb);
bool restoreFromBackup12(extFAT12_16* bpb);
bool attemptRestoreFromBackup12(extFAT12_16* bpb);
bool AnalyzeDiskData12(FILE *file, uint16_t bytesPerSec, uint8_t sectorsPerCluster, uint32_t dataStartSector, const std::vector<FAT12DirEntry>& dataDirEntries, std::vector<FileEntry> &fileEntries, bool fixErrors, bool isRootDir = true);
bool fixDataRegionErrors();
void analyzeClusterInvariants12(uint8_t* &FAT, int FATSize, int bytesPerSec, int secPerCluster, std::vector<FileEntry>& fileEntries, bool fixErrors);
void printFileInfo12( FileEntry fileEntry, FAT12DirEntry entry);

#endif //FATCHECKER_ANALYZERSFAT12_HPP