
#ifndef _GFILE_H_
#define _GFILE_H_

#include <vector>

#include "Defines.h"

struct dirent;

class GFileTree;

class GFile
{
    friend GFileTree;
public:
    GFile();
    GFile(GFile * parent_, const char * globalFullPath_, struct dirent * dirent_, long id_ = -1);
    ~GFile();
      
public:
    size_t FromBin(char * data_, char * digestBuffer_, long * parent_id_);
    size_t ToBin(char * buffer_, char * digestBuffer_);
    size_t CheckBinLength();
    
public:
    long Id();
    GFile * Parent();
    const size_t & FileSize();
    const long & MTimestamp();
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
    bool RefreshFileSize(); // return is file change.
    void AppendChild(GFile * child_);
    void RemoveChild(GFile * child_, bool bRemoveAllChildren = false);
    bool IsSamePath(GFile * file_);
    bool IsDifferent(GFile * file_);
    bool IsChild(GFile * file_);
    bool IsFile();
    bool IsFolder();
    bool IsSoftLink();
    void SortChildren();
    bool IsNeedCalSha();
    void CalShaPreInfo(char * outputLog);
    void CalSha();
    std::string ToString(bool bVerbose);
    
private:
    void ReGenerateID();
    
private:
    long    _id;
    
    GFile *             _parent;
    std::vector<GFile*> _children;
    
    char *          _globalFullPath;
    char *          _name;
    bool            _bGenShaed;
    bool            _bFile;
    bool            _bFolder;
    bool            _bSoftLink;
    size_t          _fileSize;
    long            _mTimestamp;
    unsigned char   _sha[SHA1_DIGEST_LENGTH];
    
private:
    static long _id_automatic_inc;
};

#endif /* _GFILE_H_ */
