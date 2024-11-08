//
// Created by Kateryna on 24.10.2024.
//

#ifndef FATCHECKER_FIXFAT16_HPP
#define FATCHECKER_FIXFAT16_HPP
#include "FATstructs.hpp"
bool fixBootSectorErrors(extFAT12_16* bpb);
bool fixRootDirErrors();
bool fixDataRegionErrors();
#endif //FATCHECKER_FIXFAT16_HPP
