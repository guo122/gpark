
#ifndef _GFILEMGR_H_
#define _GFILEMGR_H_

#include <map>
#include <set>

#include "Defines.h"

class GFile;
class GFileTree;

class GFileMgr
{
public:
    static GFileTree * LoadFromPath(const char * path_);
    static GFileTree * LoadFromDB(char * data_, size_t size_);
    static void LoadIgnoreFile(std::string homePath_);
    static void DifferentFileList(GFileTree * thisFileTree, GFileTree * otherFileTree,
                                  std::vector<GFile*> & changesList,
                                  std::vector<GFile*> & missList,
                                  std::vector<GFile*> & addList,
                                  std::vector<GFile*> & detailAddList);
    static void Tree(GFile * root, std::string * str, bool bVerbose, int depth = -1, std::string tab = "");
    
public:
    static unsigned long GetUUID(std::string fullPath_);
    
private:
    static void LoadFolderImpl(std::string path, GFile * parent, int & fileCount);
    
private:
    static std::map<std::string, unsigned long> _FullPathUUIDMap;
    static unsigned long _UUID_Automatic;
    
    static std::set<std::string>     _ignoreNameSet;
    static std::set<unsigned long>      _ignoreUUIDSet;
    
private:
    GFileMgr();
    ~GFileMgr();
};

#endif /* _GFILEMGR_H_ */
