
#ifndef _GTHREADHELPER_H_
#define _GTHREADHELPER_H_

#include <thread>
#include <list>

#include "Defines.h"

class GFile;

namespace GThreadHelper
{
    void PrintLog(char * log_, bool * running_);
    void PrintCalShaSize(std::vector<bool *> * threadRunningList_, std::chrono::steady_clock::time_point * time_begin_, size_t * currentSize_, const char * totalSize_, bool * running_);
    void PrintLoadFolder(std::vector<std::list<GFile *> *> * splitFileLists, std::chrono::steady_clock::time_point * time_begin_, long * currentFileCount_, bool * running_);

    void OpenFolder(const std::vector<std::list<GFile *> *> & splitFileLists, int index, long * fileCount_, long * folderCount_);
    void FileListCalSha(std::vector<GFile *> * fileList_, size_t * calculatingSize_, bool * bRunning_);
}



#endif /* _GTHREADHELPER_H_ */
