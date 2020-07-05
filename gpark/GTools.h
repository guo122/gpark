
#ifndef _GTOOLS_H_
#define _GTOOLS_H_

#include "Defines.h"

class GTools
{
public:
    static void FormatNumber(const long long & num_, char * str_);
    static void FormatFileSize(const long long & size_, char * str_);
    static void FormatFileSize(const long long & size_, char * str_, const char * color_);
    static void CalculateSHA1(const void * data_, size_t len_, unsigned char * sha_);
    static std::string FormatShaToHex(unsigned char * sha_);
    static std::string FormatTimestampToYYMMDD_HHMMSS(long timestamp_);
    
public:
    static unsigned int HardwareThreadNum();
    static unsigned int GetRecommendThreadNum(unsigned int expectThreadNum_, int ratio_);
    
private:
    GTools();
    ~GTools();
};

#endif /* _GTOOLS_H_ */
