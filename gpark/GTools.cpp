

#include "GTools.h"

void GTools::FormatFileSize(long long size_, char * str)
{
    if (size_ < STORAGE_KB)
    {
        sprintf(str, "%lld bytes", size_);
    }
    else if (size_ < STORAGE_MB)
    {
        sprintf(str, "%.2f KB", (double)size_ / (double)STORAGE_KB);
    }
    else if (size_ < STORAGE_GB)
    {
        sprintf(str, "%.2f MB", (double)size_ / (double)STORAGE_MB);
    }
    else if (size_ < STORAGE_TB)
    {
        sprintf(str, "%.2f GB", (double)size_ / (double)STORAGE_GB);
    }
    else
    {
        sprintf(str, "%.2f TB", (double)size_ / (double)STORAGE_TB);
    }
}

std::string GTools::FormatShaToHex(unsigned char * sha_)
{
    std::string ret;
    char tempChar[3];
    for (int i = 0; i < SHA_CHAR_LENGTH; i++) {
        sprintf(tempChar, "%02x", sha_[i]);
        ret += tempChar;
    }
    return ret;
}

std::string GTools::FormatTimestampToYYMMDD_HHMMSS(long timestamp_)
{
    char charBuffer[100];
    
    struct tm * tmpTm;
    time_t lt = timestamp_;
    tmpTm = localtime(&lt);
    
    strftime(charBuffer, 100, "%y%m%d_%H%M%S", tmpTm);
    
    return charBuffer;
}

GTools::GTools()
{
    
}

GTools::~GTools()
{
    
}
