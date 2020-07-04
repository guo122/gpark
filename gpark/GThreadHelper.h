
#ifndef _GTHREADHELPER_H_
#define _GTHREADHELPER_H_

#include <thread>

#include "Defines.h"

class GFile;

namespace GThreadHelper
{
    void PrintLog(char * log_, bool * running_);
    void PrintCalShaSize(unsigned int threadNum_, std::chrono::steady_clock::time_point * time_begin_, size_t * currentSize_, const char * totalSize_, bool * running_);

    void FileListCalSha(std::vector<GFile *> * fileList_, size_t * calculatingSize_);
}



#endif /* _GTHREADHELPER_H_ */
