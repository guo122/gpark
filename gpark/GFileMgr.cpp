
#include <string>
#include <fstream>
#include <list>

#include <errno.h>
#include <dirent.h>
#include <sys/types.h>

#include "GThreadHelper.h"
#include "GFile.h"
#include "GPark.h"
#include "GTools.h"
#include "GFileTree.h"

#include "GFileMgr.h"

std::set<const char *, STRCMP_Compare> GFileMgr::_GlobalFullPathSet;

std::set<std::string> GFileMgr::_ignoreNameSet;
std::set<const char *> GFileMgr::_ignoreGlobalFullPathSet;
std::set<const char *> GFileMgr::_missignoreGlobalFullPathSet;

GFileMgr::GFileMgr()
{
    
}

GFileMgr::~GFileMgr()
{
    
}

GFileTree * GFileMgr::LoadFromPath(const char * globalPath_, unsigned int threadNum_)
{
    std::chrono::steady_clock::time_point time_begin = std::chrono::steady_clock::now();
    
    threadNum_ = GTools::GetRecommendThreadNum(threadNum_, 2);
    GFile * root = new GFile(nullptr, globalPath_, nullptr);
    long fileCount = 0;
    long folderCount = 1;
    
    bool outputRunning = true;
    
    std::cout << "loading...folder (" CONSOLE_COLOR_FONT_CYAN << globalPath_ << CONSOLE_COLOR_END ")" << std::flush;
    
    std::vector<std::list<GFile *> *> splitFileLists;
    
    std::thread outputThread(GThreadHelper::PrintLoadFolder, &splitFileLists, &time_begin, &fileCount, &outputRunning);

    if (threadNum_ > 1)
    {
        std::vector<std::thread *> splitThreads;
        for (int i = 0; i < threadNum_; ++i)
        {
            std::list<GFile *> * splitFileList = new std::list<GFile *>();
            splitFileLists.push_back(splitFileList);
        }
        splitFileLists[0]->push_back(root);

        for (int i = 0; i < threadNum_; ++i)
        {
            splitThreads.push_back(new std::thread(GThreadHelper::OpenFolder, splitFileLists, i, &fileCount, &folderCount));
        }
        
        for (int i = 0; i < threadNum_; ++i)
        {
            splitThreads[i]->join();
        }
        
        for (int i = 0; i < threadNum_; ++i)
        {
            delete splitThreads[i];
            delete splitFileLists[i];
        }
    }
    else
    {
        std::list<GFile *> fileList;
        fileList.push_back(root);
        splitFileLists.push_back(&fileList);
        
        GThreadHelper::OpenFolder(splitFileLists, 0, &fileCount, &folderCount);
    }
    
    outputRunning = false;
    outputThread.join();

    std::chrono::steady_clock::time_point time_end = std::chrono::steady_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(time_end - time_begin);
    
    // todo(gzy): how cout .2f
    char tempBuffer[50];
    sprintf(tempBuffer, "%.2f", time_span.count());
    
    std::cout << CONSOLE_CLEAR_LINE "\r(" CONSOLE_COLOR_FONT_CYAN << fileCount << CONSOLE_COLOR_END " files)" CONSOLE_COLOR_FONT_YELLOW << tempBuffer << "s" CONSOLE_COLOR_END ".." CONSOLE_COLOR_FONT_GREEN "done" CONSOLE_COLOR_END << std::endl;
    
    return new GFileTree(root);
}

void GFileMgr::LoadIgnoreFile(const char * globalHomePath_)
{
    std::ifstream ifile;
    std::string homePathStr = globalHomePath_;
    
    ifile.open((homePathStr + "/" GPARK_PATH_IGNORE).c_str(), std::ios::in);
    
    if (ifile.is_open())
    {
        std::string tempStr = "";
        std::string::size_type slashPos;
        while (getline(ifile, tempStr))
        {
            if (!tempStr.empty())
            {
                slashPos = tempStr.find("/");
                if (slashPos == std::string::npos)
                {
                    _ignoreNameSet.insert(tempStr);
                }
                else if (tempStr[0] != '/')
                {
                    if (tempStr[tempStr.size() - 1] == '/')
                    {
                        tempStr.erase(tempStr.end() - 1);
                    }
                    _ignoreGlobalFullPathSet.insert(GetGlobalFullPath(globalHomePath_, tempStr.c_str()));
                }
            }
        }
        ifile.close();
    }
}

void GFileMgr::LoadMissIgnoreFile(const char * globalHomePath_)
{
    std::ifstream ifile;
    std::string homePathStr = globalHomePath_;
    
    ifile.open((homePathStr + "/" GPARK_PATH_MISS_IGNORE).c_str(), std::ios::in);
    
    if (ifile.is_open())
    {
        std::string tempStr = "";
        while (getline(ifile, tempStr))
        {
            if (!tempStr.empty() && tempStr[0] != '/')
            {
                if (tempStr[tempStr.size() - 1] == '/')
                {
                    tempStr.erase(tempStr.end() - 1);
                }
                _missignoreGlobalFullPathSet.insert(GetGlobalFullPath(globalHomePath_, tempStr.c_str()));
            }
        }
        ifile.close();
    }
}

// todo(gzy): different repos!!!
void GFileMgr::DifferentFileList(bool bMissIgnore,
                                 GFileTree * thisFileTree, GFileTree * otherFileTree,
                                 std::vector<GFile*> & changesList,
                                 std::vector<GFile*> & missList,
                                 std::vector<GFile*> & addList)
{
    if (!thisFileTree || !otherFileTree)
    {
        return;
    }
    std::vector<GFile*> folderList;
    const std::vector<GFile*> & thisFileList = thisFileTree->GetFileList();
    const std::vector<GFile*> & otherFileList = otherFileTree->GetFileList();
    
    std::set<const char *> expandMissignoreSet;
    ExpandMissIgnoreSet(thisFileTree, expandMissignoreSet);
    std::set<const char *>::iterator it_missignoreGlobal = expandMissignoreSet.end();
    
    GFile * cur = nullptr;
    bool bInFolder = false;
    for (int i = 0; i < thisFileList.size(); ++i)
    {
        cur = otherFileTree->GetFile(thisFileList[i]->GlobalFullPath());

        if (cur)
        {
            if (!cur->IsFolder() && thisFileList[i]->IsDifferent(cur))
            {
                changesList.push_back(cur);
            }
        }
        else
        {
            bInFolder = false;
            
            for (int j = 0; j < folderList.size(); ++j)
            {
                if (folderList[j]->IsChild(thisFileList[i]))
                {
                    bInFolder = true;
                    break;
                }
            }
            if (!bInFolder)
            {
                if (thisFileList[i]->IsFolder())
                {
                    folderList.push_back(thisFileList[i]);
                }
                if (bMissIgnore)
                {
                    it_missignoreGlobal = expandMissignoreSet.find(thisFileList[i]->GlobalFullPath());
                    if (it_missignoreGlobal == expandMissignoreSet.end())
                    {
                        missList.push_back(thisFileList[i]);
                    }
                }
                else
                {
                    missList.push_back(thisFileList[i]);
                }
            }
        }
    }
    folderList.clear();
    for (int i = 0; i < otherFileList.size(); ++i)
    {
        cur = thisFileTree->GetFile(otherFileList[i]->GlobalFullPath());
        
        if (!cur && otherFileList[i]->Parent() != nullptr)
        {
            bInFolder = false;
            for (int j = 0; j < folderList.size(); ++j)
            {
                if (folderList[j]->IsChild(otherFileList[i]))
                {
                    bInFolder = true;
                    break;
                }
            }
            if (!bInFolder)
            {
                if (otherFileList[i]->IsFolder())
                {
                    folderList.push_back(otherFileList[i]);
                }
                addList.push_back(otherFileList[i]);
            }
        }
    }
}

void GFileMgr::Tree(GFile * root, std::string * str, bool bVerbose, int depth, std::string tab)
{
    if (depth != 0 && depth != 1)
    {
        if (root->Parent())
        {
            *str += tab + root->ToString(bVerbose) + "\n";
            for (int i = 0; i < root->ChildrenSize(); ++i)
            {
                Tree(root->Children(i), str, bVerbose, depth - 1, tab + "|   ");
            }
        }
        else
        {
            for (int i = 0; i < root->ChildrenSize(); ++i)
            {
                Tree(root->Children(i), str, bVerbose, depth - 1, "");
            }
        }
    }
}

const char * GFileMgr::GetGlobalFullPath(const char * parentPath_, const char * name_)
{
    size_t parentPathLength = strlen(parentPath_);
    size_t nameLength = strlen(name_);
    char * ret = new char[parentPathLength + nameLength + 2];
    
    strncpy(ret, parentPath_, parentPathLength);
    strncpy(ret + parentPathLength, "/", 1);
    strncpy(ret + parentPathLength + 1, name_, nameLength + 1);
    
    auto it = _GlobalFullPathSet.find(ret);
    if (it == _GlobalFullPathSet.end())
    {
        _GlobalFullPathSet.insert(ret);
        return ret;
    }
    else
    {
        delete [] ret;
        return *it;
    }
}
const char * GFileMgr::GetGlobalFullPath(const char * fullPath_)
{
    auto it = _GlobalFullPathSet.find(fullPath_);
    if (it == _GlobalFullPathSet.end())
    {
        char * ret = new char[strlen(fullPath_) + 1];
        strcpy(ret, fullPath_);
        _GlobalFullPathSet.insert(ret);
        
        return ret;
    }
    else
    {
        return *it;
    }
}

const char * GFileMgr::GetGlobalFullPathWithIgnore(const char * parentPath_, const char * name_)
{
    const char * ret = nullptr;
    auto it_name = _ignoreNameSet.find(name_);
    if (it_name == _ignoreNameSet.end() &&
        strcmp(name_, ".") != 0 &&
        strcmp(name_, "..") != 0 &&
        strcmp(name_, GPARK_PATH_HOME) != 0)
    {
        ret = GetGlobalFullPath(parentPath_, name_);
        auto it_fullpath = _ignoreGlobalFullPathSet.find(ret);
        if (it_fullpath != _ignoreGlobalFullPathSet.end())
        {
            ret = nullptr;
        }
    }
    return ret;
}

void GFileMgr::ExpandMissIgnoreSet(GFileTree * fileTree, std::set<const char *> & expandMissignoreSet_)
{
    expandMissignoreSet_.clear();
    for (auto x : _missignoreGlobalFullPathSet)
    {
        GFile * file = fileTree->GetFile(x);
        if (file)
        {
            ExpandMissIgnoreSetImpl(file, expandMissignoreSet_);
        }
    }
}

void GFileMgr::ExpandMissIgnoreSetImpl(GFile * file_, std::set<const char *> & expandMissignoreSet_)
{
    expandMissignoreSet_.insert(file_->GlobalFullPath());
    
    for (int i = 0; i < file_->ChildrenSize(); ++i)
    {
        ExpandMissIgnoreSetImpl(file_->Children(i), expandMissignoreSet_);
    }
}
