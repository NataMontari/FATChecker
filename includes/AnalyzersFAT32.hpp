
#ifndef FATCHECKER_ANALYZERSFAT32_HPP
#define FATCHECKER_ANALYZERSFAT32_HPP


#include "FATstructs.hpp"

void AnalyzeMainFAT32();
void AnalyzeCopyFAT32();
void AnalyzeRootDir32();
void AnalyzeDiskData32();
bool isBootFAT32Invalid(extFAT32* bpb);

#endif //FATCHECKER_ANALYZERSFAT32_HPP
