
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
    static GFileTree * LoadFromPath(const char * globalPath_);
    static GFileTree * LoadFromDB(char * data_, size_t size_);
    static void LoadIgnoreFile(const char * globalHomePath_);
    static void LoadMissIgnoreFile(const char * globalHomePath_);
    static void DifferentFileList(bool bMissIgnore,
                                  GFileTree * thisFileTree, GFileTree * otherFileTree,
                                  std::vector<GFile*> & changesList,
                                  std::vector<GFile*> & missList,
                                  std::vector<GFile*> & addList);
    static void Tree(GFile * root, std::string * str, bool bVerbose, int depth = -1, std::string tab = "");
    
public:
    static const char * GetGlobalFullPath(const char * parentPath_, const char * name_);
    static const char * GetGlobalFullPath(const char * fullPath_);
    
private:
    static void ExpandMissIgnoreSet(GFileTree * fileTree, std::set<const char *> & expandMissignoreSet_);
    static void ExpandMissIgnoreSetImpl(GFile * file_, std::set<const char *> & expandMissignoreSet_);
    
private:
    static std::set<const char *, STRCMP_Compare> _GlobalFullPathSet;
    
    static std::set<std::string>     _ignoreNameSet;
    static std::set<const char *>   _ignoreGlobalFullPathSet;
    static std::set<const char *>   _missignoreGlobalFullPathSet;
    
private:
    GFileMgr();
    ~GFileMgr();
};

#endif /* _GFILEMGR_H_ */
