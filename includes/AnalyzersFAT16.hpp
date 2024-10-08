
#ifndef FATCHECKER_ANALYZERSFAT16_HPP
#define FATCHECKER_ANALYZERSFAT16_HPP

void AnalyzeMainFAT16();
void AnalyzeCopyFAT16();
void AnalyzeRootDir16();
void AnalyzeDiskData16();
bool isBootFAT16Invalid(extFAT12_16* bpb);

#endif //FATCHECKER_ANALYZERSFAT16_HPP
