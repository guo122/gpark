
//#include
#include <dirent.h>

#include "GThreadHelper.h"
#include "GFileMgr.h"

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
    void PrintLoadFolder(unsigned int threadNum_, std::chrono::steady_clock::time_point * time_begin_, long * currentFileCount_, bool * running_)
    {
        char outputLog[1024];
        while (*running_)
        {
            std::chrono::steady_clock::time_point time_end = std::chrono::steady_clock::now();
            std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(time_end - (*time_begin_));
            
            sprintf(outputLog, CONSOLE_CLEAR_LINE "\r[" CONSOLE_COLOR_FONT_PURPLE "%d" CONSOLE_COLOR_END "]loading %ld files." CONSOLE_COLOR_FONT_YELLOW "%.2fs" CONSOLE_COLOR_END, threadNum_, *currentFileCount_, time_span.count());
            std::cout << outputLog << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
}

    
    void OpenFolder(const std::vector<std::vector<GFile *> *> & fileLists, int index, long * fileCount_, long * folderCount_, const std::vector<bool *> & runningList_)
    {
        bool bContinueRunning = true;
        int threadNum = fileLists.size();
        const char * globalFullPath = nullptr;
        DIR * dir = nullptr;
        struct dirent * ptr;
        GFile * cur = nullptr;
        
        int i = 0;
        while (true)
        {
            if (i < (fileLists[index])->size())
            {
                (*fileCount_) += 1;
                (*(runningList_[index])) = true;
                dir = opendir((*(fileLists[index]))[i]->GlobalFullPath());
                if (dir)
                {
                    while ((ptr = readdir(dir)) != nullptr)
                    {
                        globalFullPath = GFileMgr::GetGlobalFullPathWithIgnore((*(fileLists[index]))[i]->GlobalFullPath(), ptr->d_name);
                        if (globalFullPath)
                        {
                            cur = new GFile((*(fileLists[index]))[i], globalFullPath, ptr);
                            (*(fileLists[index]))[i]->AppendChild(cur);
                            
                            if (cur->IsFolder())
                            {
                                fileLists[(*folderCount_) % threadNum]->push_back(cur);
                                (*folderCount_) += 1;
                            }
                        }
                    }
                    
                    (*(fileLists[index]))[i]->SortChildren();
                    closedir(dir);
                }
                else
                {
                    std::cout << "error id: [" << errno << "], can't open: (" << (*(fileLists[index]))[i]->GlobalFullPath() << ")" << std::endl;
                }
                
                ++i;
            }
            else
            {
                (*(runningList_[index])) = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                bContinueRunning = false;
                for (int i = 0; i < runningList_.size(); ++i)
                {
                    bContinueRunning |= (*(runningList_[i]));
                }
                if (!bContinueRunning)
                {
                    break;
                }
            }
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
