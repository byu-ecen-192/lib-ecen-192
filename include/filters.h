#ifndef FILTERS_H
#define FILTERS_H
#include <string>



bool processWAVFile(const std::string &inputFilePath,const std::string &outputFilePath, int cuttoff_freq,bool highPass);
bool bandRejectFilter(const std::string &inputFilePath, const std::string &outputFilePath,int freq_cut);
bool highPassFilter(const std::string &inputFilePath, const std::string &outputFilePath,int cuttoff_freq);
bool lowPassFilter(const std::string &inputFilePath, const std::string &outputFilePath,int cuttoff_freq);


#endif // FILTERS_H