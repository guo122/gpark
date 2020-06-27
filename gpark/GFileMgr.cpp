
#include <map>

#include <errno.h>
#include <dirent.h>
#include <sys/types.h>

#include "GFile.h"
#include "GPark.h"

#include "GFileMgr.h"

std::map<std::string, unsigned long> GFileMgr::_FullPathUUIDMap;
unsigned long GFileMgr::_UUID_Automatic = 0;

GFileMgr::GFileMgr()
{
    
}

GFileMgr::~GFileMgr()
{
    
}

GFile * GFileMgr::LoadFromPath(const char * path_)
{
    GFile * ret = new GFile(nullptr, path_, nullptr);
    int fileCount = 0;
    
    std::cout << "load...";
    LoadFolderImpl(path_, ret, fileCount);
    std::cout << "done(" << fileCount << ")" << std::endl;
    
    return ret;
}


GFile * GFileMgr::Load(char * data_, size_t size_)
{
    GFile * root = nullptr;
    GFile * parent = nullptr;
    GFile * cur = nullptr;
    long parent_id = -122;
    
    std::map<long, GFile*> gfileMap;
    
    size_t offset = 0, size = 0;
    while (offset < size_)
    {
        size = *((size_t*)(data_ + offset)) + DB_OFFSET_LENGTH;

        cur = new GFile(data_ + offset, size, parent_id);
        if (root == nullptr)
        {
            root = new GFile(nullptr, GPark::Instance()->GetHomePath().c_str(), nullptr, parent_id);
            gfileMap.insert(std::pair<long, GFile*>(parent_id, root));
        }
        
        parent = gfileMap[parent_id];
        
        if (parent == nullptr)
        {
            GAssert(parent, "can't find parent id when load DB.");
        }

        parent->AppendChild(cur);
        cur->GenFullPath();
        
        gfileMap.insert(std::pair<long, GFile*>(cur->Id(), cur));
        
        offset += size;
    }
    
    return root;
}

void GFileMgr::DifferentFileList(GFile * savedFileRoot_, GFile * nowFileRoot_,
                                 std::vector<GFile*> & changesList,
                                 std::vector<GFile*> & missList,
                                 std::vector<GFile*> & addList,
                                 std::vector<GFile*> & detailAddList)
{
    std::vector<GFile *> savedFileList, nowFileList, folderList;
    GetFileList(savedFileRoot_, savedFileList);
    GetFileList(nowFileRoot_, nowFileList);
    
    std::map<unsigned long, GFile *> savedFileMap;
    std::map<unsigned long, GFile *> nowFileMap;
    
    for (int i = 0; i < savedFileList.size(); ++i)
    {
        savedFileMap.insert(std::pair<unsigned long, GFile *>(savedFileList[i]->FullPathUUID(), savedFileList[i]));
    }
    
    for (int i = 0; i < nowFileList.size(); ++i)
    {
        nowFileMap.insert(std::pair<unsigned long, GFile *>(nowFileList[i]->FullPathUUID(), nowFileList[i]));
    }
    std::map<unsigned long, GFile *>::iterator it;
    
    GFile * cur = nullptr;
    bool bInFolder = false;
    for (int i = 0; i < savedFileList.size(); ++i)
    {
        it = nowFileMap.find(savedFileList[i]->FullPathUUID());
        if (it == nowFileMap.end())
        {
            cur = nullptr;
        }
        else
        {
            cur = it->second;
        }
//        for (int ii = 0; ii < nowFileList.size(); ++ii)
//        {
//            if (nowFileList[ii]->IsSamePath(savedFileList[i]))
//            {
//                cur = nowFileList[ii];
//                break;
//            }
//        }
        //         cur = GFileMgr::GetFile(nowFileRoot_, savedFileList[i]->FullPathUUID());
        if (cur)
        {
            if (!cur->IsFolder() && cur->IsDifferent(savedFileList[i]))
            {
                changesList.push_back(cur);
            }
        }
        else
        {
            bInFolder = false;
            
            for (int j = 0; j < folderList.size(); ++j)
            {
                if (folderList[j]->IsChild(savedFileList[i]))
                {
                    bInFolder = true;
                    break;
                }
            }
            if (!bInFolder)
            {
                if (savedFileList[i]->IsFolder())
                {
                    folderList.push_back(savedFileList[i]);
                }
                missList.push_back(savedFileList[i]);
            }
        }
    }
    folderList.clear();
    for (int i = 0; i < nowFileList.size(); ++i)
    {
        it = savedFileMap.find(nowFileList[i]->FullPathUUID());
        if (it == savedFileMap.end())
        {
            cur = nullptr;
        }
        else
        {
            cur = it->second;
        }
//        for (int ii = 0; ii < savedFileList.size(); ++ii)
//        {
//            if (savedFileList[ii]->IsSamePath(nowFileList[i]))
//            {
//                cur = savedFileList[ii];
//                break;
//            }
//        }
        //         cur = GFileMgr::GetFile(savedFileRoot_, nowFileList[i]->FullPathUUID());
        if (!cur && nowFileList[i]->Parent() != nullptr)
        {
            bInFolder = false;
            for (int j = 0; j < folderList.size(); ++j)
            {
                if (folderList[j]->IsChild(nowFileList[i]))
                {
                    bInFolder = true;
                    break;
                }
            }
            if (!bInFolder)
            {
                if (nowFileList[i]->IsFolder())
                {
                    folderList.push_back(nowFileList[i]);
                }
                addList.push_back(nowFileList[i]);
            }
            detailAddList.push_back(nowFileList[i]);
        }
    }
}

void GFileMgr::Tree(GFile * root, std::string * str, bool bVerbose, std::string tab)
{
    if (root->Parent())
    {
        *str += tab + root->ToString(bVerbose) + "\n";
        for (int i = 0; i < root->ChildrenSize(); ++i)
        {
            Tree(root->Children(i), str, bVerbose, tab + "|   ");
        }
    }
    else
    {
        for (int i = 0; i < root->ChildrenSize(); ++i)
        {
            Tree(root->Children(i), str, bVerbose, "");
        }
    }
}

void GFileMgr::CheckSize(GFile * root, size_t & size)
{
    if (root->Parent())
    {
        size += root->SaveSize() + DB_OFFSET_LENGTH;
    }
    
    for (int i = 0; i < root->ChildrenSize(); ++i)
    {
        CheckSize(root->Children(i), size);
    }
}

void GFileMgr::ToBin(GFile * root, char * data_, size_t & offset_)
{
    if (root->Parent())
    {
        size_t size = root->ToBin(data_, offset_);
        offset_ += size;
    }
    
    for (int i = 0; i < root->ChildrenSize(); ++i)
    {
        ToBin(root->Children(i), data_, offset_);
    }
}

GFile * GFileMgr::GetFile(GFile * root, const unsigned long fullPathUUID_)
{
    GFile * ret = nullptr;
    
    if (root->FullPathUUID() == fullPathUUID_)
    {
        ret = root;
    }
    else
    {
        for (int i = 0; i < root->ChildrenSize(); ++i)
        {
            ret = GetFile(root->Children(i), fullPathUUID_);
            if (ret)
            {
                break;
            }
        }
    }

    return ret;
}

void GFileMgr::GetFileList(GFile * root_, std::vector<GFile *> & fileList_)
{
    fileList_.push_back(root_);
    
    for (int i = 0; i < root_->ChildrenSize(); ++i)
    {
        GetFileList(root_->Children(i), fileList_);
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
        struct dirent * ptr;
        GFile * currentFile = nullptr;
        while ((ptr = readdir(dir)) != nullptr)
        {
            if (strcmp(ptr->d_name, ".") != 0 &&
                strcmp(ptr->d_name, "..") != 0 &&
                strcmp(ptr->d_name, ".DS_Store") != 0 &&
                strcmp(ptr->d_name, ".git") != 0 &&
                strcmp(ptr->d_name, REPOS_PATH_FOLDER) != 0)
            {
                fileCount++;
                std::string fileFullPatn = path + "/" + ptr->d_name;
                currentFile = new GFile(parent, fileFullPatn.c_str(), ptr);
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
