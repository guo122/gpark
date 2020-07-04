
#include "GThreadHelper.h"

#include "GFile.h"

namespace GThreadHelper
{
    void PrintLog(char * log_, bool * running_)
    {
        while (*running_)
        {
            std::cout << log_ << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }

    void FileListCalSha(std::vector<GFile *> * fileList_)
    {
        char outputLog[1024];
        bool * outputRunning = new bool;
        *outputRunning = true;
        std::thread outputThread(PrintLog, outputLog, outputRunning);
        for (int i = 0; i < fileList_->size(); ++i)
        {
            (*fileList_)[i]->CalShaPreInfo(outputLog);
            (*fileList_)[i]->CalSha();
        }
        *outputRunning = false;
        outputThread.join();
        delete outputRunning;
    }
}
