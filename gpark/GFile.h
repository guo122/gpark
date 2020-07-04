
#ifndef _GFILE_H_
#define _GFILE_H_

#include <vector>
#include <sys/stat.h>

#include "Defines.h"

struct dirent;

class GFileTree;

class GFile
{
    friend GFileTree;
public:
    GFile(char * data_, size_t size_, long & parent_id_);
    GFile(GFile * parent_, const char * globalFullPath_, struct dirent * dirent_, long id_ = -1);
    ~GFile();
    
public:
    long Id();
    GFile * Parent();
    struct stat & Stat();
    const char * CurrentPath();
    const char * GlobalFullPath();
    const char * Name();
    unsigned char * Sha();
    size_t ChildrenSize();
    GFile* Children(int index_);
    
public:
    void SetParent(GFile * parent_);
    void CopyFrom(GFile * file_);
    void GenFullPath();
    void AppendChild(GFile * child_);
    void RemoveChild(GFile * child_, bool bRemoveAllChildren = false);
    bool IsSamePath(GFile * file_);
    bool IsDifferent(GFile * file_);
    bool IsChild(GFile * file_);
    bool IsFolder();
    void SortChildren();
    bool IsNeedCalSha();
    void CalShaPreInfo(char * outputLog);
    void CalSha();
    std::string ToString(bool bVerbose);
    size_t ToBin(char * data_, size_t offset_);
    
    size_t SaveSize();
    
private:
    void ReGenerateID();
    
private:
    long    _id;
    
    GFile *             _parent;
    std::vector<GFile*> _children;
    
    struct stat     _stat;
    
    char *          _globalFullPath;
    char *          _name;
    bool            _bGenShaed;
    bool            _bFolder;
    unsigned char   _sha[SHA_CHAR_LENGTH];
    
private:
    static long _id_automatic_inc;
};

#endif /* _GFILE_H_ */
