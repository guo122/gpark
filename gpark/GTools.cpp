

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

GTools::GTools()
{
    
}

GTools::~GTools()
{
    
}
