
#ifndef FATCHECKER_ANALYZERSFAT16_HPP
#define FATCHECKER_ANALYZERSFAT16_HPP

bool analyzeFAT16Tables(const std::vector<uint16_t*>& FATs, int FATSize, uint16_t bytesPerSec, bool fixErrors);
void AnalyzeCopyFAT16();
bool AnalyzeRootDir16(const std::vector<FAT16DirEntry>& rootDirEntries, std::vector<FAT16DirEntry>& dataDirEntries, std::vector<FileEntry> &fileEntries, bool fixErrors);
bool AnalyzeDiskData16(FILE *file, uint16_t bytesPerSec, uint8_t sectorsPerCluster, uint32_t dataStartSector, const std::vector<FAT16DirEntry>& dataDirEntries, std::vector<FileEntry> &fileEntries, bool fixErrors, bool isRootDir = true);
void analyzeClusterInvariants(uint16_t* FAT, int FATSize, int bytesPerSec, int secPerCluster, std::vector<FileEntry> &fileEntries, bool fixErrors);
bool isBootFAT16Invalid(extFAT12_16* bpb, bool fixErrors);
void PrintFileEntries(const std::vector<FileEntry>& fileEntries);
std::vector<uint16_t> loadFAT16Table(const uint8_t* fatBuffer, int fatSizeBytes);

#endif //FATCHECKER_ANALYZERSFAT16_HPP
