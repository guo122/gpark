
#include "GThreadHelper.h"
#include "GFile.h"
#include "GTools.h"

#include "GFileTree.h"

GFileTree::GFileTree(GFile * root_)
    : _root(root_)
{
    Refresh(true);
}

GFileTree::~GFileTree()
{
    
}

GFile * GFileTree::Root()
{
    return _root;
}

GFile * GFileTree::GetFile(const char * globalFullPath_)
{
    GFile * ret = nullptr;
    
    auto it = _fileMap.find(globalFullPath_);
    if (it != _fileMap.end())
    {
        ret = it->second;
    }
    
    return ret;
}

GFileTree * GFileTree::GetSubTree(const char * globalFullPath_)
{
    GFileTree * ret = nullptr;
    
    auto it = _fileMap.find(globalFullPath_);
    if (it != _fileMap.end())
    {
        ret = new GFileTree(it->second);
    }
    
    return ret;
}

const std::vector<GFile*> & GFileTree::GetFileList()
{
    return _fileList;
}

size_t GFileTree::CheckBinLength()
{
    size_t ret = 0;
    
    for (int i = 1; i < _fileList.size(); ++i)
    {
        ret += _fileList[i]->CheckBinLength();
    }
    
    return ret;
}

void GFileTree::ToBin(char * buffer_, char * digestBuffer_, unsigned int threadNum_)
{
    std::chrono::steady_clock::time_point time_begin = std::chrono::steady_clock::now();
    
    auto fileListSize = _fileList.size();
    char outputLog[1024];
    bool outputRunning = true;
    std::vector<bool *> calShaThreadRunningList;
    
    long currentFileCount = 0;
    size_t currentCalShaSize = 0;
    size_t totalNeedShaSize = 0;
    std::vector<GFile *> sortList;
    threadNum_ = GTools::GetRecommendThreadNum(threadNum_, 1);
    for (int i = 0; i < fileListSize; ++i)
    {
        if (_fileList[i]->IsNeedCalSha())
        {
            totalNeedShaSize += _fileList[i]->FileSize();
            sortList.push_back(_fileList[i]);
        }
    }
    std::sort(sortList.begin(), sortList.end(), [](GFile * & x, GFile * & y){
        if (x->IsNeedCalSha() && !y->IsNeedCalSha()) return true;
        else if (!x->IsNeedCalSha() && y->IsNeedCalSha()) return false;
        else if (x->FileSize() > y->FileSize()) return true;
        else if (x->FileSize() < y->FileSize()) return false;
        else return false;
    });
    std::vector<std::thread *> threadList;
    
    outputRunning = true;
    char totalFileCountBuf[50];
    GTools::FormatNumber(sortList.size(), totalFileCountBuf);
    GTools::FormatFileSize(totalNeedShaSize, outputLog);
    std::thread calShaLogThread(GThreadHelper::PrintCalShaSize, &calShaThreadRunningList, &time_begin, &currentCalShaSize, outputLog, &currentFileCount,totalFileCountBuf, &outputRunning);
    if (threadNum_ > 1 && sortList.size() > 1)
    {
        if (threadNum_ > sortList.size())
        {
            threadNum_ = sortList.size();
        }
        std::vector<std::vector<GFile *> *> splitFileLists;
        std::vector<size_t> splitListSize;
        std::thread * splitThread;
        
        std::cout << "cal sha" << std::flush;
        
        for (int i = 0; i < threadNum_; ++i)
        {
            // todo(gzy): need opt.
            std::vector<GFile *> * splitFileList = new std::vector<GFile *>();
            splitFileLists.push_back(splitFileList);
            splitListSize.push_back(0);
            calShaThreadRunningList.push_back(new bool);
        }
        
        bool bForBigFileThread = sortList[0]->FileSize() > BIG_FILE_SIZE;
        int currentShorterIndex = 0;
        size_t currentShorterSize = totalNeedShaSize;
        for (int i = 0; i < sortList.size(); ++i)
        {
            if (bForBigFileThread && sortList[i]->FileSize() > BIG_FILE_SIZE)
            {
                currentShorterIndex = 0;
            }
            else
            {
                currentShorterIndex = 1;
            }
            if (currentShorterIndex != 0)
            {
                currentShorterSize = totalNeedShaSize;
                for (int j = bForBigFileThread; j < threadNum_; ++j)
                {
                    if (splitListSize[j] < currentShorterSize)
                    {
                        currentShorterSize = splitListSize[j];
                        currentShorterIndex = j;
                    }
                }
            }
            
            splitFileLists[currentShorterIndex]->push_back(sortList[i]);
            splitListSize[currentShorterIndex] += sortList[i]->FileSize();
        }
        
        for (int i = 0; i < threadNum_; ++i)
        {
            splitThread = new std::thread(GThreadHelper::FileListCalSha, splitFileLists[i], &currentFileCount, &currentCalShaSize, calShaThreadRunningList[i]);
            threadList.push_back(splitThread);
        }
        
        for (int i = 0; i < threadNum_; ++i)
        {
            threadList[i]->join();
        }
        
        for (int i = 0; i < threadNum_; ++i)
        {
            delete threadList[i];
            delete splitFileLists[i];
            delete calShaThreadRunningList[i];
        }
    }
    else
    {
        calShaThreadRunningList.push_back(new bool);
        GThreadHelper::FileListCalSha(&sortList, &currentFileCount, &currentCalShaSize, calShaThreadRunningList[0]);
        delete calShaThreadRunningList[0];
    }
    outputRunning = false;
    calShaLogThread.join();
    
    std::chrono::steady_clock::time_point time_end = std::chrono::steady_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(time_end - time_begin);
    
    char timeSpanBuf[30];
    GTools::FormatTimeduration(time_span.count(), timeSpanBuf);
    std::cout << CONSOLE_CLEAR_LINE "\rcal sha (" CONSOLE_COLOR_FONT_CYAN << outputLog << CONSOLE_COLOR_END ")" CONSOLE_COLOR_FONT_YELLOW << timeSpanBuf <<  CONSOLE_COLOR_END ".." CONSOLE_COLOR_FONT_GREEN "done" CONSOLE_COLOR_END << std::endl;
    
    size_t offset = 0;
    outputRunning = true;
    std::thread toBinLogThread(GThreadHelper::PrintLog, outputLog, &outputRunning);
    for (int i = 1; i < fileListSize; ++i)
    {
        sprintf(outputLog, CONSOLE_CLEAR_LINE "\rto bin (" CONSOLE_COLOR_FONT_CYAN "%d/%ld" CONSOLE_COLOR_END ")", i, fileListSize - 1);
        offset += _fileList[i]->ToBin(buffer_ + offset, digestBuffer_ + offset);
    }
    outputRunning = false;
    toBinLogThread.join();
    std::cout << CONSOLE_CLEAR_LINE "\rto bin (" CONSOLE_COLOR_FONT_CYAN << fileListSize - 1 << CONSOLE_COLOR_END ").." CONSOLE_COLOR_FONT_GREEN "done" CONSOLE_COLOR_END << std::endl;
}

void GFileTree::Refresh(bool bRefreshID)
{
    _fileList.clear();
    GenFileList(_root);
    
    _fileMap.clear();
    for (int i = 0; i < _fileList.size(); ++i)
    {
        _fileMap.insert(std::pair<const char *, GFile *>(_fileList[i]->GlobalFullPath(), _fileList[i]));
    }
    
    if (bRefreshID)
    {
        for (int i = 0; i < _fileList.size(); ++i)
        {
            _fileList[i]->ReGenerateID();
        }
    }
}

void GFileTree::GenFileList(GFile * file_)
{
    _fileList.push_back(file_);
    
    for (int i = 0; i < file_->ChildrenSize(); ++i)
    {
        GenFileList(file_->Children(i));
    }
}
