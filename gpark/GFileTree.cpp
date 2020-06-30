
#include "GFile.h"

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

void GFileTree::ToBin(char * data_)
{
    size_t offset = 0;
    for (int i = 1; i < _fileList.size(); ++i)
    {
        offset += _fileList[i]->ToBin(data_, offset);
    }
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
