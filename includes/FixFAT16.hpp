//
// Created by Kateryna on 24.10.2024.
//

#ifndef FATCHECKER_FIXFAT16_HPP
#define FATCHECKER_FIXFAT16_HPP
#include "FATstructs.hpp"


struct BootSectorErrors {
    bool BPB_JumpAddressInvalid = false;
    bool BPB_BytsPerSecInvalid = false;
    bool BPB_SecPerClusInvalid = false;
    bool BPB_RsvdSecCntInvalid = false;
    bool BPB_NumFATsInvalid = false;
    bool BPB_FATSz16Invalid = false;
    bool BPB_RootEntCntInvalid = false;
    bool BPB_MediaInvalid = false;
    bool BPB_SectorsPerTrackInvalid = false;
    bool BPB_NumHeadsInvalid = false;
    bool BPB_TotSec16Invalid = false;
    bool BPB_NumFATsTooMuchInvalid = false;
    bool BPB_DrvNumInvalid = false;
    bool BPB_BS_ReservedInvalid = false;
    bool BPB_BootSigInvalid = false;

};

bool fixBootSectorErrors(extFAT12_16* bpb, BootSectorErrors &errors);
bool fixRootDirErrors();
bool fixDataRegionErrors();

#endif //FATCHECKER_FIXFAT16_HPP
