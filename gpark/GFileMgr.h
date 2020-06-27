
#ifndef _GFILEMGR_H_
#define _GFILEMGR_H_

#include "Defines.h"

class GFile;

class GFileMgr
{
public:
    static GFile * LoadFromPath(const std::string &path_);
    static GFile * Load(char * data_, size_t size_);
    static void DifferentFileList(GFile * savedFileRoot_, GFile * nowFileRoot_,
                                  std::vector<GFile*> & changesList,
                                  std::vector<GFile*> & missList,
                                  std::vector<GFile*> & addList);
    static void Tree(GFile * root, std::string * str, std::string tab = "");
    static void CheckSize(GFile * root, size_t & size);
    static void ToBin(GFile * root, char * data_, size_t & offset_);
    
public:
    static GFile * GetFile(GFile * root, const std::string & fullPath_);
    static void GetFileList(GFile * root_, std::vector<GFile *> & fileList_);
    
private:
    static void LoadFolderImpl(std::string path, GFile * parent, int & fileCount);
    
private:
    GFileMgr();
    ~GFileMgr();
};

#endif /* _GFILEMGR_H_ */
