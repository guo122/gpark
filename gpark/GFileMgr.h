
#ifndef _GFILEMGR_H_
#define _GFILEMGR_H_

#include <map>

#include "Defines.h"

class GFile;

class GFileMgr
{
public:
    static GFile * LoadFromPath(const char * path_);
    static GFile * Load(char * data_, size_t size_);
    static void DifferentFileList(GFile * savedFileRoot_, GFile * nowFileRoot_,
                                  std::vector<GFile*> & changesList,
                                  std::vector<GFile*> & missList,
                                  std::vector<GFile*> & addList,
                                  std::vector<GFile*> & detailAddList);
    static void Tree(GFile * root, std::string * str, bool bVerbose, std::string tab = "");
    static void CheckSize(GFile * root, size_t & size);
    static void ToBin(GFile * root, char * data_, size_t & offset_);
    
public:
    static GFile * GetFile(GFile * root, const unsigned long fullPathUUID_);
    static void GetFileList(GFile * root_, std::vector<GFile *> & fileList_);
    
public:
    static unsigned long GetUUID(std::string fullPath_);
    
private:
    static void LoadFolderImpl(std::string path, GFile * parent, int & fileCount);
    
private:
    static std::map<std::string, unsigned long> _FullPathUUIDMap;
    static unsigned long _UUID_Automatic;
    
private:
    GFileMgr();
    ~GFileMgr();
};

#endif /* _GFILEMGR_H_ */
