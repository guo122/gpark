
#include <string>
#include <fstream>

#include <errno.h>
#include <dirent.h>
#include <sys/types.h>

#include "GFile.h"
#include "GPark.h"
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

GFileTree * GFileMgr::LoadFromPath(const char * globalPath_)
{
    GFile * root = new GFile(nullptr, globalPath_, nullptr);
    int fileCount = 0;
    
    std::cout << "loading...folder (" CONSOLE_COLOR_FONT_CYAN << globalPath_ << CONSOLE_COLOR_END ")" << std::flush;
    std::vector<GFile *> cacheFileList;
    cacheFileList.push_back(root);
    
    DIR * dir = nullptr;
    std::set<std::string>::iterator it_name = _ignoreNameSet.end();
    std::set<const char *>::iterator it_fullpath = _ignoreGlobalFullPathSet.end();
    struct dirent * ptr;
    GFile * parent = nullptr;
    GFile * cur = nullptr;
    const char * globalFullPath = nullptr;
    
    std::string::size_type cacheIndex = 0;
    while (cacheIndex < cacheFileList.size())
    {
        parent = cacheFileList[cacheIndex];
        dir = opendir(parent->GlobalFullPath());
        
        if (dir)
        {
            while ((ptr = readdir(dir)) != nullptr)
            {
                it_name = _ignoreNameSet.find(ptr->d_name);
                if (it_name != _ignoreNameSet.end())
                {
                    continue;
                }
                
                if (strcmp(ptr->d_name, ".") != 0 &&
                    strcmp(ptr->d_name, "..") != 0 &&
                    strcmp(ptr->d_name, GPARK_PATH_HOME) != 0)
                {
                    globalFullPath = GetGlobalFullPath(parent->GlobalFullPath(), ptr->d_name);

                    it_fullpath = _ignoreGlobalFullPathSet.find(globalFullPath);
                    
                    if (it_fullpath != _ignoreGlobalFullPathSet.end())
                    {
                        continue;
                    }
                    
                    fileCount++;
                    
                    cur = new GFile(parent, globalFullPath, ptr);
                    parent->AppendChild(cur);
                    
                    if (cur->IsFolder())
                    {
                        cacheFileList.push_back(cur);
                    }
                }
            }
            
            parent->SortChildren();
            closedir(dir);
        }
        else
        {
            std::cout << "error id: [" << errno << "], can't open: (" << parent->GlobalFullPath() << ")" << std::endl;
        }
        
        ++cacheIndex;
    }
    
    std::cout << "(" CONSOLE_COLOR_FONT_CYAN << fileCount << CONSOLE_COLOR_END " files)..done" << std::endl;
    
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
