
#ifndef FATCHECKER_ANALYZERSFAT16_HPP
#define FATCHECKER_ANALYZERSFAT16_HPP

bool analyzeFAT16Tables(const std::vector<uint16_t*>& FATs, int FATSize, uint16_t bytesPerSec);
void AnalyzeCopyFAT16();
bool AnalyzeRootDir16(const std::vector<FAT16DirEntry>& rootDirEntries, std::vector<FAT16DirEntry>& dataDirEntries);
bool AnalyzeDiskData16(int fd, uint16_t bytesPerSec, uint8_t sectorsPerCluster, uint32_t dataStartSector, const std::vector<FAT16DirEntry>& dataDirEntries);
bool isBootFAT16Invalid(extFAT12_16* bpb);

#endif //FATCHECKER_ANALYZERSFAT16_HPP
