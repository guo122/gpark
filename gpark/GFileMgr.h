
#ifndef _GFILEMGR_H_
#define _GFILEMGR_H_

#include <map>

#include "Defines.h"

class GFile;
class GFileTree;

class GFileMgr
{
public:
    static GFileTree * LoadFromPath(const char * path_);
    static GFileTree * LoadFromDB(char * data_, size_t size_);
    static void DifferentFileList(GFileTree * thisFileTree, GFileTree * otherFileTree,
                                  std::vector<GFile*> & changesList,
                                  std::vector<GFile*> & missList,
                                  std::vector<GFile*> & addList,
                                  std::vector<GFile*> & detailAddList);
    static void Tree(GFile * root, std::string * str, bool bVerbose, std::string tab = "");
    
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
