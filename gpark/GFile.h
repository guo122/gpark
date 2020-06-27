
#ifndef _GFILE_H_
#define _GFILE_H_

#include <vector>
#include <sys/stat.h>

#include "Defines.h"

struct dirent;

class GFile
{
public:
    GFile(char * data_, size_t size_, long & parent_id_);
    GFile(GFile * parent_, const std::string & fullPath_ = "", struct dirent * dirent_ = nullptr, long id_ = -1);
    ~GFile();
    
public:
    long Id();
    GFile * Parent();
    struct stat & Stat();
    const char * CurrentPath();
    const std::string & FullPath();
    const std::string & Name();
    unsigned char * Sha();
    size_t ChildrenSize();
    GFile* Children(int index_);
    
public:
    void SetParent(GFile * parent_);
    void CopyFrom(GFile * file_);
    void AppendChild(GFile * child_);
    void RemoveChild(GFile * child_, bool bRemoveAllChildren = false);
    bool IsDifferent(GFile * file_);
    bool IsChild(GFile * file_);
    bool IsFolder();
    void SortChildren();
    std::string ToString();
    size_t ToBin(char * data_, size_t offset_);
    
    size_t SaveSize();
    
private:
    long    _id;
    
    GFile *             _parent;
    std::vector<GFile*> _children;
    
    struct stat     _stat;
    
    std::string     _fullPath;
    std::string     _name;
    bool            _bGenShaed;
    unsigned char   _sha[SHA_CHAR_LENGTH];
    
private:
    static long _id_automatic_inc;
};

#endif /* _GFILE_H_ */
