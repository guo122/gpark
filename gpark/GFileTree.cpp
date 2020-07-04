
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

size_t GFileTree::CheckSize()
{
    size_t ret = 0;
    
    for (int i = 1; i < _fileList.size(); ++i)
    {
        ret += _fileList[i]->SaveSize() + DB_OFFSET_LENGTH;
    }
    
    return ret;
}

void GFileTree::ToBin(char * data_, char * totalSha_, unsigned int threadNum_)
{
    std::chrono::steady_clock::time_point time_begin = std::chrono::steady_clock::now();
    
    auto fileListSize = _fileList.size();
    char outputLog[1024];
    bool * outputRunning = new bool;
    
    size_t currentCalShaSize = 0;
    size_t totalNeedShaSize = 0;
    std::vector<GFile *> sortList;
    unsigned int hardwareThreadNum = GTools::HardwareThreadNum();
    if (threadNum_ == 0 || threadNum_ > hardwareThreadNum)
    {
        threadNum_ = hardwareThreadNum / 2;
    }
    for (int i = 0; i < fileListSize; ++i)
    {
        if (_fileList[i]->IsNeedCalSha())
        {
            totalNeedShaSize += _fileList[i]->Stat().st_size;
            sortList.push_back(_fileList[i]);
        }
    }
    std::sort(sortList.begin(), sortList.end(), [](GFile * & x, GFile * & y){
        if (x->IsNeedCalSha() && !y->IsNeedCalSha()) return true;
        else if (!x->IsNeedCalSha() && y->IsNeedCalSha()) return false;
        else if (x->Stat().st_size > y->Stat().st_size) return true;
        else if (x->Stat().st_size < y->Stat().st_size) return false;
        else return false;
    });
    
    *outputRunning = true;
    GTools::FormatFileSize(totalNeedShaSize, outputLog, CONSOLE_COLOR_FONT_CYAN);
    std::thread calShaLogThrad(GThreadHelper::PrintCalShaSize, threadNum_, &time_begin, &currentCalShaSize, outputLog, outputRunning);
    if (threadNum_ > 1 && sortList.size() > 1)
    {
        if (threadNum_ > sortList.size())
        {
            threadNum_ = sortList.size();
        }
        std::vector<std::vector<GFile *> *> splitFileLists;
        std::vector<size_t> splitListSize;
        std::vector<std::thread *> threadList;
        std::thread * splitThread;
        
        std::cout << "cal sha" << std::flush;
        
        for (int i = 0; i < threadNum_; ++i)
        {
            // todo(gzy): need opt.
            std::vector<GFile *> * splitFileList = new std::vector<GFile *>();
            splitFileLists.push_back(splitFileList);
            splitListSize.push_back(0);
        }
        
        int currentShorterIndex = 0;
        size_t currentShorterSize = totalNeedShaSize;
        for (int i = 0; i < sortList.size(); ++i)
        {
            currentShorterIndex = 0;
            currentShorterSize = totalNeedShaSize;
            for (int j = 0; j < threadNum_; ++j)
            {
                if (splitListSize[j] < currentShorterSize)
                {
                    currentShorterSize = splitListSize[j];
                    currentShorterIndex = j;
                }
            }
            
            splitFileLists[currentShorterIndex]->push_back(sortList[i]);
            splitListSize[currentShorterIndex] += sortList[i]->Stat().st_size;
        }
        
        for (int i = 0; i < threadNum_; ++i)
        {
            splitThread = new std::thread(GThreadHelper::FileListCalSha, splitFileLists[i], &currentCalShaSize);
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
        }
    }
    else
    {
        GThreadHelper::FileListCalSha(&sortList, &currentCalShaSize);
    }
    *outputRunning = false;
    calShaLogThrad.join();
    
    std::chrono::steady_clock::time_point time_end = std::chrono::steady_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(time_end - time_begin);
    
    // todo(gzy): how cout .2f
    char tempBuffer[50];
    sprintf(tempBuffer, "%.2f", time_span.count());
    std::cout << CONSOLE_CLEAR_LINE "\rcal sha (" << outputLog << ")" CONSOLE_COLOR_FONT_YELLOW << tempBuffer << "s" <<  CONSOLE_COLOR_END ".." CONSOLE_COLOR_FONT_GREEN "done" CONSOLE_COLOR_END << std::endl;
    
    size_t offset = 0;
    *outputRunning = true;
    std::thread toBinLogThread(GThreadHelper::PrintLog, outputLog, outputRunning);
    for (int i = 1; i < fileListSize; ++i)
    {
        sprintf(outputLog, CONSOLE_CLEAR_LINE "\rto bin (" CONSOLE_COLOR_FONT_CYAN "%d/%ld" CONSOLE_COLOR_END ")", i, fileListSize - 1);
        offset += _fileList[i]->ToBin(data_, offset);
        memcpy(totalSha_ + ((i - 1) * SHA_CHAR_LENGTH), _fileList[i]->Sha(), SHA_CHAR_LENGTH);
    }
    *outputRunning = false;
    toBinLogThread.join();
    delete outputRunning;
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
