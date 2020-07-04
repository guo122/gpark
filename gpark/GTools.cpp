
#include <thread>

#include <CommonCrypto/CommonDigest.h>
//#include <openssl/sha.h>

#include "3rd/sha1.h"

#include "GTools.h"

void GTools::FormatFileSize(long long size_, char * str, const char * color_)
{
    if (size_ < STORAGE_KB)
    {
        sprintf(str, "%s%lld bytes" CONSOLE_COLOR_END, color_, size_);
    }
    else if (size_ < STORAGE_MB)
    {
        sprintf(str, "%s%.2f KB" CONSOLE_COLOR_END, color_, (double)size_ / (double)STORAGE_KB);
    }
    else if (size_ < STORAGE_GB)
    {
        sprintf(str, "%s%.2f MB" CONSOLE_COLOR_END, color_, (double)size_ / (double)STORAGE_MB);
    }
    else if (size_ < STORAGE_TB)
    {
        sprintf(str, "%s%.2f GB" CONSOLE_COLOR_END, color_, (double)size_ / (double)STORAGE_GB);
    }
    else
    {
        sprintf(str, "%s%.2f TB" CONSOLE_COLOR_END, color_, (double)size_ / (double)STORAGE_TB);
    }
}

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

void GTools::CalculateSHA1(const void * data_, size_t len_, unsigned char * sha_)
{
//    SHA_CTX ctx;
//    SHA1_Init(&ctx);
//    SHA1_Update(&ctx, data_, len_);
//    SHA1_Final(sha_, &ctx);
    
//    blk_SHA_CTX ctx;
//    blk_SHA1_Init(&ctx);
//    blk_SHA1_Update(&ctx, data_, len_);
//    blk_SHA1_Final(sha_, &ctx);
    
    // todo(gzy): platform?
    CC_SHA1_CTX ctx;
    CC_SHA1_Init(&ctx);
    CC_SHA1_Update(&ctx, data_, len_);
    CC_SHA1_Final(sha_, &ctx);
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

unsigned int GTools::HardwareThreadNum()
{
    int ret = std::thread::hardware_concurrency();
    return ret;
}

GTools::GTools()
{
    
}

GTools::~GTools()
{
    
}
