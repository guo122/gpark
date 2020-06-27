
#ifndef _GFILEMGR_H_
#define _GFILEMGR_H_

#include "Defines.h"

class GFile;

class GFileMgr
{
public:
    static GFile * Load();
    static GFile * Load(char * data_, size_t size_);
    static void Tree(GFile * root, std::string * str, std::string tab = "");
    static void CheckSize(GFile * root, size_t & size);
    static void ToBin(GFile * root, char * data_, size_t & offset_);
    
private:
    static void LoadFolderImpl(std::string path, GFile * parent);
    
private:
    GFileMgr();
    ~GFileMgr();
};

#endif /* _GFILEMGR_H_ */
