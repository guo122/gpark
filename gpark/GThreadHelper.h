
#ifndef _GTHREADHELPER_H_
#define _GTHREADHELPER_H_

#include <thread>

#include "Defines.h"

class GFile;

namespace GThreadHelper
{
    void PrintLog(char * log_, bool * running_);
    void PrintCalShaSize(unsigned int threadNum_, std::chrono::steady_clock::time_point * time_begin_, size_t * currentSize_, const char * totalSize_, bool * running_);
    void PrintLoadFolder(unsigned int threadNum_, std::chrono::steady_clock::time_point * time_begin_, long * currentFileCount_, bool * running_);

    void OpenFolder(const std::vector<std::vector<GFile *> *> & fileLists, int index, long * fileCount_, long * folderCount_, const std::vector<bool *> & runningList_);
    void FileListCalSha(std::vector<GFile *> * fileList_, size_t * calculatingSize_);
}



#endif /* _GTHREADHELPER_H_ */
