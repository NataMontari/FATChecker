//
// Created by Kateryna on 09.10.2024.
//

#ifndef FATCHECKER_ANALYZERSFAT12_HPP
#define FATCHECKER_ANALYZERSFAT12_HPP

void AnalyzeMainFAT12();
void AnalyzeCopyFAT12();
void AnalyzeRootDir12();
void AnalyzeDiskData12();
bool isBootFAT12Invalid(extFAT12_16* bpb);

#endif //FATCHECKER_ANALYZERSFAT12_HPP
