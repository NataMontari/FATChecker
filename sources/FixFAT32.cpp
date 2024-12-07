#include <iostream>
#include "FixFAT32.hpp"
#include <cstring>

bool fixBootSectorErrorsFAT32(extFAT32* bpb, BootSectorErrorsFAT32 &errors) {
    bool isFixed = false;

    std::cout << "Attempting to fix FAT32 boot sector errors..." << std::endl;

    if (errors.BPB_JumpAddressInvalid) {
        bpb->basic.BS_jmpBoot[0] = 0xEB;
        bpb->basic.BS_jmpBoot[2] = 0x90;
        std::cout << "Fixed: Set BS_jmpBoot to standard jump code." << std::endl;
        isFixed = true;
    }

    if (errors.BPB_BytsPerSecInvalid) {
        bpb->basic.BPB_BytsPerSec = 512;
        std::cout << "Fixed: Set BPB_BytsPerSec to 512." << std::endl;
        isFixed = true;
    }

    if (errors.BPB_SecPerClusInvalid) {
        bpb->basic.BPB_SecPerClus = 8; // typical for FAT32
        std::cout << "Fixed: Set BPB_SecPerClus to 8." << std::endl;
        isFixed = true;
    }

    if (errors.BPB_RsvdSecCntInvalid) {
        bpb->basic.BPB_RsvdSecCnt = 32; // standard for FAT32
        std::cout << "Fixed: Set BPB_RsvdSecCnt to 32." << std::endl;
        isFixed = true;
    }

    if (errors.BPB_NumFATsInvalid) {
        bpb->basic.BPB_NumFATs = 2;
        std::cout << "Fixed: Set BPB_NumFATs to 2." << std::endl;
        isFixed = true;
    }

    if (errors.BPB_RootEntCntInvalid) {
        bpb->basic.BPB_RootEntCnt = 0;
        std::cout << "Fixed: Set BPB_RootEntCnt to 0 (FAT32 requirement)." << std::endl;
        isFixed = true;
    }

    if (errors.BPB_TotSec16Invalid) {
        bpb->basic.BPB_TotSec16 = 0;
        std::cout << "Fixed: Set BPB_TotSec16 to 0 (FAT32 requirement)." << std::endl;
        isFixed = true;
    }

    if (errors.BPB_TotSec32Invalid) {
        bpb->basic.BPB_TotSec32 = 100000; // Exs
        std::cout << "Fixed: Set BPB_TotSec32 to 100000." << std::endl;
        isFixed = true;
    }

    if (errors.BPB_FATSz32Invalid) {
        bpb->BPB_FATSz32 = 8192; // Ex
        std::cout << "Fixed: Set BPB_FATSz32 to 8192 sectors." << std::endl;
        isFixed = true;
    }

    if (errors.BPB_MediaInvalid) {
        bpb->basic.BPB_Media = 0xF8;
        std::cout << "Fixed: Set BPB_Media to 0xF8." << std::endl;
        isFixed = true;
    }

    if (errors.BPB_SectorsPerTrackInvalid) {
        bpb->basic.BPB_SecPerTrk = 63;
        std::cout << "Fixed: Set BPB_SecPerTrk to 63." << std::endl;
        isFixed = true;
    }

    if (errors.BPB_NumHeadsInvalid) {
        bpb->basic.BPB_NumHeads = 255;
        std::cout << "Fixed: Set BPB_NumHeads to 255." << std::endl;
        isFixed = true;
    }

    if (errors.BPB_RootClusInvalid) {
        bpb->BPB_RootClus = 2; // Standard starting root cluster
        std::cout << "Fixed: Set BPB_RootClus to 2." << std::endl;
        isFixed = true;
    }

    if (errors.BPB_BootSigInvalid) {
        bpb->BS_BootSig = 0x29;
        std::cout << "Fixed: Set BS_BootSig to 0x29." << std::endl;
        isFixed = true;
    }

    if (errors.BPB_DrvNumInvalid) {
        bpb->BS_DrvNum = 0x80;
        std::cout << "Fixed: Set BS_DrvNum to 0x80." << std::endl;
        isFixed = true;
    }

    if (errors.BPB_BS_ReservedInvalid) {
        bpb->BS_Reserved1 = 0x00;
        std::cout << "Fixed: Set BS_Reserved1 to 0." << std::endl;
        isFixed = true;
    }

    return isFixed;
}

bool fixRootDirErrorsFAT32() {
    std::cout << "Attempting to fix FAT32 root directory errors..." << std::endl;
    // todo
    return true;
}

bool fixDataRegionErrorsFAT32() {
    std::cout << "Attempting to fix FAT32 data region errors..." << std::endl;
    //todo
    return true;
}

bool fixClusterErrorsFAT32() {
    std::cout << "Attempting to fix FAT32 cluster errors..." << std::endl;
    //todo
    return true;
}
