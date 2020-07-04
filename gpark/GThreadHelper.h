
#ifndef _GTHREADHELPER_H_
#define _GTHREADHELPER_H_

#include <thread>

#include "Defines.h"

class GFile;

namespace GThreadHelper
{
    void PrintLog(char * log_, bool * running_);

    void FileListCalSha(std::vector<GFile *> * fileList_);
}



#endif /* _GTHREADHELPER_H_ */
