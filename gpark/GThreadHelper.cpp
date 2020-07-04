
#include "GThreadHelper.h"

#include "GTools.h"
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
    void PrintCalShaSize(unsigned int threadNum_, std::chrono::steady_clock::time_point * time_begin_, size_t * currentSize_, const char * totalSize_, bool * running_)
    {
        char sizeFormatBuf[FORMAT_FILESIZE_BUFFER_LENGTH];
        char outputLog[1024];
        while (*running_)
        {
            std::chrono::steady_clock::time_point time_end = std::chrono::steady_clock::now();
            std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(time_end - (*time_begin_));
            
            GTools::FormatFileSize(*currentSize_, sizeFormatBuf, CONSOLE_COLOR_FONT_CYAN);
            sprintf(outputLog, CONSOLE_CLEAR_LINE "\r[" CONSOLE_COLOR_FONT_PURPLE "%d" CONSOLE_COLOR_END "]cal sha (%s/%s)" CONSOLE_COLOR_FONT_YELLOW "%.2fs" CONSOLE_COLOR_END, threadNum_, sizeFormatBuf, totalSize_, time_span.count());
            std::cout << outputLog << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }

    void FileListCalSha(std::vector<GFile *> * fileList_, size_t * calculatingSize_)
    {
        for (int i = 0; i < fileList_->size(); ++i)
        {
            (*fileList_)[i]->CalSha();
            (*calculatingSize_) += (*fileList_)[i]->Stat().st_size;
        }
    }
}
