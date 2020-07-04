
#include <dirent.h>
#include <mutex>

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
    void PrintCalShaSize(std::vector<bool *> * threadRunningList_, std::chrono::steady_clock::time_point * time_begin_, size_t * currentSize_, const char * totalSize_, bool * running_)
    {
        char sizeFormatBuf[FORMAT_FILESIZE_BUFFER_LENGTH];
        char outputLog[1024];
        int threadNum = 1;
        while (*running_)
        {
            threadNum = 0;
            for (int i = 0; i < threadRunningList_->size(); ++i)
            {
                if (*((*threadRunningList_)[i]))
                {
                    threadNum++;
                }
            }
            std::chrono::steady_clock::time_point time_end = std::chrono::steady_clock::now();
            std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(time_end - (*time_begin_));
            
            GTools::FormatFileSize(*currentSize_, sizeFormatBuf, CONSOLE_COLOR_FONT_CYAN);
            sprintf(outputLog, CONSOLE_CLEAR_LINE "\r[" CONSOLE_COLOR_FONT_PURPLE "%d" CONSOLE_COLOR_END "]cal sha (%s/%s)" CONSOLE_COLOR_FONT_YELLOW "%.2fs" CONSOLE_COLOR_END, threadNum, sizeFormatBuf, totalSize_, time_span.count());
            std::cout << outputLog << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }
    void PrintLoadFolder(std::vector<std::list<GFile *> *> * splitFileLists, std::chrono::steady_clock::time_point * time_begin_, long * currentFileCount_, bool * running_)
    {
        char outputLog[1024];
        int threadNum = 1;
        while (*running_)
        {
            threadNum = 0;
            for (int i = 0; i < splitFileLists->size(); ++i)
            {
                if ((*splitFileLists)[i]->begin() != (*splitFileLists)[i]->end())
                {
                    threadNum++;
                }
            }
            std::chrono::steady_clock::time_point time_end = std::chrono::steady_clock::now();
            std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(time_end - (*time_begin_));
            
            sprintf(outputLog, CONSOLE_CLEAR_LINE "\r[" CONSOLE_COLOR_FONT_PURPLE "%d" CONSOLE_COLOR_END "]loading %ld files." CONSOLE_COLOR_FONT_YELLOW "%.2fs" CONSOLE_COLOR_END, threadNum, *currentFileCount_, time_span.count());
            std::cout << outputLog << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }

    std::mutex mutexOpenFolderFileCount;
    std::mutex mutexOpenFolerSplitFileLists;
    
    void OpenFolder(const std::vector<std::list<GFile *> *> & splitFileLists, int index, long * fileCount_, long * folderCount_)
    {
        bool bContinueRunning = true;
        int threadNum = splitFileLists.size();
        const char * globalFullPath = nullptr;
        DIR * dir = nullptr;
        struct dirent * ptr;
        GFile * cur = nullptr;
        std::list<GFile *>::iterator it = splitFileLists[index]->begin();
        
        int nextThreadIndex = 0;
        
        while (true)
        {
            if (it != splitFileLists[index]->end())
            {
                dir = opendir((*it)->GlobalFullPath());
                if (dir)
                {
                    while ((ptr = readdir(dir)) != nullptr)
                    {
                        globalFullPath = GFileMgr::GetGlobalFullPathWithIgnore((*it)->GlobalFullPath(), ptr->d_name);
                        if (globalFullPath)
                        {
                            cur = new GFile((*it), globalFullPath, ptr);
                            (*it)->AppendChild(cur);

                            mutexOpenFolderFileCount.lock();
                            (*fileCount_) += 1;
                            mutexOpenFolderFileCount.unlock();
                            
                            if (cur->IsFolder())
                            {
                                mutexOpenFolerSplitFileLists.lock();
                                nextThreadIndex = (*folderCount_) % threadNum;
                                splitFileLists[nextThreadIndex]->push_back(cur);
                                (*folderCount_) += 1;
                                mutexOpenFolerSplitFileLists.unlock();
                            }
                        }
                    }
                    
                    (*it)->SortChildren();
                    closedir(dir);
                }
                else
                {
                    std::cout << "error id: [" << errno << "], can't open: (" << (*it)->GlobalFullPath() << ")" << std::endl;
                }
                
                mutexOpenFolerSplitFileLists.lock();
                splitFileLists[index]->erase(it);
                it = splitFileLists[index]->begin();
                mutexOpenFolerSplitFileLists.unlock();
            }
            else
            {
                mutexOpenFolerSplitFileLists.lock();
                it = splitFileLists[index]->begin();
                mutexOpenFolerSplitFileLists.unlock();
                
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                bContinueRunning = false;
                for (int i = 0; i < splitFileLists.size(); ++i)
                {
                    bContinueRunning |= splitFileLists[i]->begin() != splitFileLists[i]->end();
                }
                if (!bContinueRunning)
                {
                    break;
                }
            }
        }
    }

    void FileListCalSha(std::vector<GFile *> * fileList_, size_t * calculatingSize_, bool * bRunning_)
    {
        for (int i = 0; i < fileList_->size(); ++i)
        {
            (*bRunning_) = true;
            (*fileList_)[i]->CalSha();
            (*calculatingSize_) += (*fileList_)[i]->Stat().st_size;
        }
        (*bRunning_) = false;
    }
}
