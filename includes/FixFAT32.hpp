#ifndef FATCHECKER_FIXFAT32_HPP
#define FATCHECKER_FIXFAT32_HPP
#include "FATstructs.hpp"

struct BootSectorErrorsFAT32 {
    bool BPB_JumpAddressInvalid = false;
    bool BPB_BytsPerSecInvalid = false;
    bool BPB_SecPerClusInvalid = false;
    bool BPB_RsvdSecCntInvalid = false;
    bool BPB_NumFATsInvalid = false;
    bool BPB_RootEntCntInvalid = false;
    bool BPB_TotSec16Invalid = false;
    bool BPB_TotSec32Invalid = false;
    bool BPB_FATSz32Invalid = false;
    bool BPB_MediaInvalid = false;
    bool BPB_SectorsPerTrackInvalid = false;
    bool BPB_NumHeadsInvalid = false;
    bool BPB_RootClusInvalid = false;
    bool BPB_BootSigInvalid = false;
    bool BPB_DrvNumInvalid = false;
    bool BPB_BS_ReservedInvalid = false;
};


bool fixBootSectorErrorsFAT32(extFAT32* bpb, BootSectorErrorsFAT32 &errors);
bool fixRootDirErrorsFAT32();
bool fixDataRegionErrorsFAT32();
bool fixClusterErrorsFAT32();

#endif //FATCHECKER_FIXFAT32_HPP
