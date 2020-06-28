
#ifndef _GFILETREE_H_
#define _GFILETREE_H_

#include <map>
#include <vector>

#include "Defines.h"

class GFile;

class GFileTree
{
public:
    GFileTree(GFile * root_);
    ~GFileTree();
    
public:
    GFile * Root();
    GFile * GetFile(const unsigned long & fullPathUUID_);
    GFileTree * GetSubTree(const unsigned long & fullPathUUID_);
    const std::vector<GFile*> & GetFileList();
    
public:
    size_t CheckSize();
    void ToBin(char * data_);
    
public:
    void Refresh(bool bRefreshID);
    
private:
    void GenFileList(GFile * file_);
    
private:
    GFile * _root;
    std::vector<GFile*> _fileList;
    std::map<unsigned long, GFile*> _fileMap;
};

#endif /* _GFILETREE_H_ */