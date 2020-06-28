
#include <string>
#include <fstream>

#include <errno.h>
#include <dirent.h>
#include <sys/types.h>

#include "GFile.h"
#include "GPark.h"
#include "GFileTree.h"

#include "GFileMgr.h"

std::map<std::string, unsigned long> GFileMgr::_FullPathUUIDMap;
unsigned long GFileMgr::_UUID_Automatic = 0;

std::set<std::string> GFileMgr::_ignoreNameSet;
std::set<unsigned long> GFileMgr::_ignoreUUIDSet;

GFileMgr::GFileMgr()
{
    
}

GFileMgr::~GFileMgr()
{
    
}

GFileTree * GFileMgr::LoadFromPath(const char * path_)
{
    GFile * root = new GFile(nullptr, path_, GetUUID(path_), nullptr);
    int fileCount = 0;
    
    std::cout << "load...";
    LoadFolderImpl(path_, root, fileCount);
    std::cout << "done(" << fileCount << ")" << std::endl;
    
    return new GFileTree(root);
}


GFileTree * GFileMgr::LoadFromDB(char * data_, size_t size_)
{
    GFile * root = nullptr;
    GFile * parent = nullptr;
    GFile * cur = nullptr;
    long parent_id = -122;
    
    std::map<long, GFile*> gfileMap;
    std::map<long, GFile*>::iterator it;
    
    size_t offset = 0, size = 0;
    while (offset < size_)
    {
        size = *((size_t*)(data_ + offset)) + DB_OFFSET_LENGTH;

        cur = new GFile(data_ + offset, size, parent_id);
        if (root == nullptr)
        {
            root = new GFile(nullptr, GPark::Instance()->GetHomePath().c_str(), GetUUID(GPark::Instance()->GetHomePath()), nullptr, parent_id);
            gfileMap.insert(std::pair<long, GFile*>(parent_id, root));
        }
        
        parent = gfileMap[parent_id];
        
        GAssert(parent, "can't find parent id when load DB.");

        parent->AppendChild(cur);
        cur->GenFullPath();
        
        it = gfileMap.find(cur->Id());
        GAssert(it == gfileMap.end(), "same id %ld (%s)", cur->Id(), cur->FullPath());
        
        gfileMap.insert(std::pair<long, GFile*>(cur->Id(), cur));
        
        offset += size;
    }
    
    return new GFileTree(root);
}

void GFileMgr::LoadIgnoreFile(std::string homePath_)
{
    std::ifstream ifile;
    
    ifile.open((homePath_ + "/" GPARK_PATH_IGNORE).c_str(), std::ios::in);
    
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
                    tempStr = homePath_ + "/" + tempStr;
                    _ignoreUUIDSet.insert(GetUUID(tempStr));
                }
            }
        }
        ifile.close();
    }
}

void GFileMgr::DifferentFileList(GFileTree * thisFileTree, GFileTree * otherFileTree,
                                 std::vector<GFile*> & changesList,
                                 std::vector<GFile*> & missList,
                                 std::vector<GFile*> & addList,
                                 std::vector<GFile*> & detailAddList)
{
    std::vector<GFile*> folderList;
    const std::vector<GFile*> & thisFileList = thisFileTree->GetFileList();
    const std::vector<GFile*> & otherFileList = otherFileTree->GetFileList();
    
    GFile * cur = nullptr;
    bool bInFolder = false;
    for (int i = 0; i < thisFileList.size(); ++i)
    {
        cur = otherFileTree->GetFile(thisFileList[i]->FullPathUUID());

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
                missList.push_back(thisFileList[i]);
            }
        }
    }
    folderList.clear();
    for (int i = 0; i < otherFileList.size(); ++i)
    {
        cur = thisFileTree->GetFile(otherFileList[i]->FullPathUUID());
        
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
            detailAddList.push_back(otherFileList[i]);
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

unsigned long GFileMgr::GetUUID(std::string fullPath_)
{
    std::map<std::string, unsigned long>::iterator it = _FullPathUUIDMap.find(fullPath_);
    if (it == _FullPathUUIDMap.end())
    {
        _FullPathUUIDMap.insert(std::pair<std::string, unsigned long>(fullPath_, _UUID_Automatic++));
        it = _FullPathUUIDMap.find(fullPath_);
    }
    
    return it->second;
}

void GFileMgr::LoadFolderImpl(std::string path, GFile * parent, int & fileCount)
{
    DIR * dir = opendir(path.c_str());
    
    if (dir)
    {
        std::set<std::string>::iterator it_name = _ignoreNameSet.end();
        std::set<unsigned long>::iterator it_UUID = _ignoreUUIDSet.end();
        struct dirent * ptr;
        GFile * currentFile = nullptr;
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
                std::string fileFullPatn = path + "/" + ptr->d_name;
                unsigned long fullPathUUID = GetUUID(fileFullPatn);
                
                it_UUID = _ignoreUUIDSet.find(fullPathUUID);
                
                if (it_UUID != _ignoreUUIDSet.end())
                {
                    continue;
                }
                
                fileCount++;
                
                currentFile = new GFile(parent, fileFullPatn.c_str(), fullPathUUID, ptr);
                parent->AppendChild(currentFile);
                
                if (currentFile->IsFolder())
                {
                    LoadFolderImpl(fileFullPatn.c_str(), currentFile, fileCount);
                }
            }
        }
        
        parent->SortChildren();
    }
    else
    {
        std::cout << "error id: [" << errno << "] can't open: (" << path << ")" << std::endl;
    }
    
    closedir(dir);
}
