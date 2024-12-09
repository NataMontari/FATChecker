
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

#endif //FATCHECKER_READFUNCTIONS_HPP
