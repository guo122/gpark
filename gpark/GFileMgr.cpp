
#include <map>

#include <dirent.h>
#include <sys/types.h>

#include "GFile.h"
#include "GPark.h"

#include "GFileMgr.h"

GFileMgr::GFileMgr()
{
    
}

GFileMgr::~GFileMgr()
{
    
}

GFile * GFileMgr::Load()
{
    GFile * ret = new GFile(nullptr);
    
    LoadFolderImpl(GPark::Instance()->GetWorkPath(), ret);
    
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
        
        if (parent_id == -122)
        {
            cur = new GFile(data_ + offset, size, parent_id);
            root = new GFile(nullptr, "", nullptr, parent_id);
            gfileMap.insert(std::pair<long, GFile*>(parent_id, root));
        }
        else
        {
            cur = new GFile(data_ + offset, size, parent_id);
        }
        
        parent = gfileMap[parent_id];
        
        GAssert(parent, "can't find parent id when load DB.");

        parent->AppendChildren(cur);
        
        gfileMap.insert(std::pair<long, GFile*>(cur->Id(), cur));
        
        offset += size;
    }
    
    
    
    return root;
}

void GFileMgr::Tree(GFile * root, std::string * str, std::string tab)
{
    if (root->Parent())
    {
        *str += tab + root->ToString() + "\n";
        for (int i = 0; i < root->ChildrenSize(); ++i)
        {
            Tree(root->Children(i), str, tab + "|   ");
        }
    }
    else
    {
        for (int i = 0; i < root->ChildrenSize(); ++i)
        {
            Tree(root->Children(i), str, "");
        }
    }
}

void GFileMgr::CheckSize(GFile * root, size_t & size)
{
    if (root->Parent())
    {
        size += root->Size() + DB_OFFSET_LENGTH;
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

void GFileMgr::LoadFolderImpl(std::string path, GFile * parent)
{
    DIR * dir = opendir(path.c_str());
    struct dirent * ptr;
    
    GFile * currentFile = nullptr;
    while ((ptr = readdir(dir)) != nullptr)
    {
        if (strcmp(ptr->d_name, ".") != 0 &&
            strcmp(ptr->d_name, "..") != 0 &&
            strcmp(ptr->d_name, ".DS_Store") != 0)
        {
            currentFile = new GFile(parent, path + "/" + ptr->d_name, ptr);
            parent->AppendChildren(currentFile);
            
            if (currentFile->IsFolder())
            {
                LoadFolderImpl((path + "/" + ptr->d_name).c_str(), currentFile);
            }
        }
    }
    
    parent->SortChildren();
 
}
