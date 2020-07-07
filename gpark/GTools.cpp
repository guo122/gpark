
#include <thread>

//#include <CommonCrypto/CommonDigest.h>
//#include <openssl/sha.h>

#include "3rd/sha1.h"

#include "GTools.h"

void GTools::FormatNumber(const long long & num_, char * str_)
{
    char tempChar[30];
    sprintf(tempChar, "%lld", num_);
    int tempCharLen = strlen(tempChar);
    int needAppend = 0;
    if (tempCharLen > 3)
    {
        needAppend = (tempCharLen - 1) / 3;
        for (int i = 0, tempI = tempCharLen - 1, strI = tempCharLen + needAppend - 1; tempI >= 0 && strI >= 0; --tempI, --strI)
        {
            str_[strI] = tempChar[tempI];
            if (i == 2)
            {
                str_[--strI] = ',';
                i = 0;
            }
            else
            {
                ++i;
            }
        }
    }
    else
    {
        strcpy(str_, tempChar);
    }
    str_[tempCharLen + needAppend] = 0;
}

void GTools::FormatFileSize(const long long & size_, char * str_, const char * color_)
{
    if (size_ < STORAGE_KB)
    {
        sprintf(str_, "%s%lld bytes" CONSOLE_COLOR_END, color_, size_);
    }
    else if (size_ < STORAGE_MB)
    {
        sprintf(str_, "%s%.2f KB" CONSOLE_COLOR_END, color_, (double)size_ / (double)STORAGE_KB);
    }
    else if (size_ < STORAGE_GB)
    {
        sprintf(str_, "%s%.2f MB" CONSOLE_COLOR_END, color_, (double)size_ / (double)STORAGE_MB);
    }
    else if (size_ < STORAGE_TB)
    {
        sprintf(str_, "%s%.2f GB" CONSOLE_COLOR_END, color_, (double)size_ / (double)STORAGE_GB);
    }
    else
    {
        sprintf(str_, "%s%.2f TB" CONSOLE_COLOR_END, color_, (double)size_ / (double)STORAGE_TB);
    }
}

void GTools::FormatFileSize(const long long & size_, char * str_)
{
    if (size_ < STORAGE_KB)
    {
        sprintf(str_, "%lld bytes", size_);
    }
    else if (size_ < STORAGE_MB)
    {
        sprintf(str_, "%.2f KB", (double)size_ / (double)STORAGE_KB);
    }
    else if (size_ < STORAGE_GB)
    {
        sprintf(str_, "%.2f MB", (double)size_ / (double)STORAGE_MB);
    }
    else if (size_ < STORAGE_TB)
    {
        sprintf(str_, "%.2f GB", (double)size_ / (double)STORAGE_GB);
    }
    else
    {
        sprintf(str_, "%.2f TB", (double)size_ / (double)STORAGE_TB);
    }
}

void GTools::CalculateSHA1(const void * data_, size_t len_, unsigned char * sha_)
{
//    SHA_CTX ctx;
//    SHA1_Init(&ctx);
//    SHA1_Update(&ctx, data_, len_);
//    SHA1_Final(sha_, &ctx);
    
    blk_SHA_CTX ctx;
    blk_SHA1_Init(&ctx);
    blk_SHA1_Update(&ctx, data_, len_);
    blk_SHA1_Final(sha_, &ctx);
    
    // todo(gzy): int32???
//    CC_SHA1_CTX ctx;
//    CC_SHA1_Init(&ctx);
//    CC_SHA1_Update(&ctx, data_, len_);
//    CC_SHA1_Final(sha_, &ctx);
}

std::string GTools::FormatShaToHex(unsigned char * sha_)
{
    std::string ret;
    char tempChar[3];
    for (int i = 0; i < SHA1_DIGEST_LENGTH; i++) {
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

void GTools::FormatTimeduration(const double & duration_, char * str_)
{
    sprintf(str_, "%.2fs", duration_);
//    long totalSecond = duration_;
//    if (totalSecond >= 60)
//    {
//        int len = strlen(str_);
//        char decimal[3], hourBuf[10], minBuf[6], secBuf[6];
//        decimal[0] = str_[len - 2];
//        decimal[1] = str_[len - 1];
//        decimal[2] = 0;
//
//        int hour = totalSecond / 3600;
//        int min = totalSecond % 3600 / 60;
//        int sec = totalSecond % 60;
//
//        if (hour > 0)
//        {
//            sprintf(hourBuf, "%dh", hour);
//            sprintf(minBuf, "00m");
//        }
//        else
//        {
//            hourBuf[0] = 0;
//            minBuf[0] = 0;
//        }
//
//        if (min > 0 && min < 10)
//        {
//            sprintf(minBuf, "0%dm", min);
//        }
//        else if (min >= 10)
//        {
//            sprintf(minBuf, "%dm", min);
//        }
//
//        if (sec == 0)
//        {
//            sprintf(secBuf, "00");
//        }
//        else if (sec < 10)
//        {
//            sprintf(secBuf, "0%d", sec);
//        }
//        else
//        {
//            sprintf(secBuf, "%d", sec);
//        }
//
//        sprintf(str_, "%s%s%s.%ss", hourBuf, minBuf, secBuf, decimal);
//    }
}

unsigned int GTools::HardwareThreadNum()
{
    unsigned int ret = std::thread::hardware_concurrency();
    return ret;
}

unsigned int GTools::GetRecommendThreadNum(unsigned int expectThreadNum_, int ratio_)
{
    unsigned int ret = std::thread::hardware_concurrency();
    if (expectThreadNum_ == 0 || expectThreadNum_ > ret)
    {
        ret = (ret / 2) * ratio_;
    }
    else
    {
        ret = expectThreadNum_;
    }
    return ret;
}

GTools::GTools()
{
    
}

GTools::~GTools()
{
    
}
