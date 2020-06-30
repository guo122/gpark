
#ifndef _GTOOLS_H_
#define _GTOOLS_H_

#include "Defines.h"

class GTools
{
public:
    static void FormatFileSize(long long size_, char * str);
    static void FormatFileSize(long long size_, char * str, const char * color_);
    static std::string FormatShaToHex(unsigned char * sha_);
    static std::string FormatTimestampToYYMMDD_HHMMSS(long timestamp_);
    
private:
    GTools();
    ~GTools();
};

#endif /* _GTOOLS_H_ */
