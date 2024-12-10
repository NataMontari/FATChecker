
#ifndef FATCHECKER_READFUNCTIONS_HPP
#define FATCHECKER_READFUNCTIONS_HPP

#include <cstdint>
#include "FATstructs.hpp"
#include "file.hpp"
#include <iostream>
#include <cstring>

bool readBackupBootSector(uint8_t *bootSector, int bootSector_size);
bool writeBootSectorToFile(const extFAT12_16* bpb);
void writeFATTableToFile(uint16_t* FAT, int FATIndex, uint16_t bytesPerSec, int FATSize, int startFATSector);
bool writeBootSectorToFile32(const extFAT32* bpb);
bool readBackupBootSector32(uint8_t* bootSector, int bootSector_size);
void writeFATTableToFile32(uint32_t* FAT, int FATIndex, uint16_t bytesPerSec, int FATSize, int startFATSector);
void writeFATTableToFile12(uint8_t* FAT, int FATIndex, uint16_t bytesPerSec, int FATSize, int startFATSector);
bool WriteRootDirToImage(const std::vector<FAT16DirEntry>& rootDirEntries, uint32_t rootDirSector, uint16_t bytesPerSec) ;
void updateMultipleFATCopies(const uint16_t* FAT, int FATSize, int numCopies, int startFATAdress, size_t copyOffset);
void updateMultipleFATCopies32(const uint32_t* FAT, int FATSize, int numCopies, int startFATAdress, size_t copyOffset);
void updateMultipleFATCopies12(const uint8_t* FAT, int FATSize, int numCopies, int startFATAdress, size_t copyOffset) ;

#endif //FATCHECKER_READFUNCTIONS_HPP
