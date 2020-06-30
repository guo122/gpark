
#include <fstream>
#include <dirent.h>
#include <openssl/sha.h>

#include "GTools.h"
#include "GPark.h"
#include "GFileMgr.h"
#include "GFileTree.h"

#include "GFile.h"

long GFile::_id_automatic_inc = 0;

GFile::GFile(char * data_, size_t size_, long & parent_id_)
    : _id(0)
    , _parent(nullptr)
    , _fullPath(nullptr)
    , _name(nullptr)
    , _bGenShaed(false)
    , _FullPathUUID(0)
{
    size_t nameLength = size_ - SaveSize() - DB_OFFSET_LENGTH;
    char * buffer = new char[size_];
    
    data_ += DB_OFFSET_LENGTH;
    
    memcpy(buffer, data_, sizeof(long));
    data_ += sizeof(long);
    _id = *((long*)buffer);
    
    memcpy(buffer, data_, sizeof(long));
    data_ += sizeof(long);
    parent_id_ = *((long*)buffer);
    
    memcpy(buffer, data_, sizeof(struct stat));
    data_ += sizeof(struct stat);
    _stat = *((struct stat*)buffer);
    
    memcpy(buffer, data_, nameLength + 1);
    data_ += nameLength + 1;
    _name = new char[nameLength + 1];
    strncpy(_name, buffer, nameLength);
    _name[nameLength] = 0;
    
    memcpy(buffer, data_, SHA_CHAR_LENGTH);
    data_ += SHA_CHAR_LENGTH;
    memcpy(_sha, buffer, SHA_CHAR_LENGTH);
    
    delete [] buffer;
}

GFile::GFile(GFile * parent_, const char * fullPath_, const unsigned long & fullPathUUID_, struct dirent * dirent_, long id_)
    : _id(id_)
    , _parent(parent_)
    , _fullPath(nullptr)
    , _name(nullptr)
    , _bGenShaed(false)
    , _FullPathUUID(fullPathUUID_)
{
    memset(_sha, 0, SHA_CHAR_LENGTH);
    size_t fullPathLength = strlen(fullPath_);
    _fullPath = new char[fullPathLength + 1];
    strncpy(_fullPath, fullPath_, fullPathLength);
    _fullPath[fullPathLength] = 0;
    
    if (_id == -1)
    {
        _id = ++_id_automatic_inc;
    }
    
    if (dirent_)
    {
        _name = new char[dirent_->d_namlen + 1];
        strncpy(_name, dirent_->d_name, dirent_->d_namlen);
        _name[dirent_->d_namlen] = 0;
    }
    stat(_fullPath, &_stat);
}

GFile::~GFile()
{
    if (_name)
    {
        delete [] _name;
    }
    if (_fullPath)
    {
        delete [] _fullPath;
    }
}

long GFile::Id()
{
    return _id;
}

GFile * GFile::Parent()
{
    return _parent;
}
struct stat & GFile::Stat()
{
    return _stat;
}
const char * GFile::CurrentPath()
{
    if (GPark::Instance()->GetWorkPath().size() >= strlen(_fullPath))
    {
        return "./";
    }
    else
    {
        return _fullPath + GPark::Instance()->GetWorkPath().size() + 1;
    }
}
const char * GFile::FullPath()
{
    return _fullPath;
}
const char * GFile::Name()
{
    return _name;
}
const unsigned long & GFile::FullPathUUID()
{
    return _FullPathUUID;
}

unsigned char * GFile::Sha()
{
    if (!_bGenShaed)
    {
        _bGenShaed = true;
        
        GAssert(_fullPath, "has no full path.");
        
        if (_name != nullptr && !IsFolder())
        {
            char sizeBuf[FORMAT_FILESIZE_BUFFER_LENGTH];
            GTools::FormatFileSize(_stat.st_size, sizeBuf);
            std::cout << "calculate sha (" << sizeBuf << ") " << _name << ".  " << std::flush;
            
            SHA_CTX ctx;
            std::ifstream ifile;
            ifile.open(_fullPath, std::ios::in | std::ios::binary);
            char * buffer = new char[_stat.st_size];
            ifile.read(buffer, _stat.st_size);
            
            SHA1_Init(&ctx);
            SHA1_Update(&ctx, buffer, _stat.st_size);
            SHA1_Final(_sha, &ctx);
            
            ifile.close();
            delete[] buffer;
            
            std::cout << "\rdone" << std::flush;
        }
    }
    return _sha;
}

size_t GFile::ChildrenSize()
{
    return _children.size();
}
GFile* GFile::Children(int index_)
{
    GAssert(index_ >= 0 && index_ < _children.size(), "get GFile Children index out of range.");
    
    return _children[index_];
}

void GFile::SetParent(GFile * parent_)
{
    _parent = parent_;
}

void GFile::CopyFrom(GFile * file_)
{
    _bGenShaed = file_->_bGenShaed;
    memcpy(_sha, file_->Sha(), SHA_CHAR_LENGTH);
    _stat = file_->Stat();
}

void GFile::GenFullPath()
{
    if (_fullPath)
    {
        delete [] _fullPath;
    }
    size_t parentfullPathLength = strlen(_parent->FullPath());
    size_t nameLength = strlen(_name);
    _fullPath = new char[parentfullPathLength + nameLength + 2];
    strncpy(_fullPath, _parent->FullPath(), parentfullPathLength);
    strncpy(_fullPath + parentfullPathLength, "/", 1);
    strncpy(_fullPath + parentfullPathLength + 1, _name, nameLength);
    _fullPath[parentfullPathLength + nameLength + 1] = 0;
    
    _FullPathUUID = GFileMgr::GetUUID(_fullPath);
}

void GFile::AppendChild(GFile * child_)
{
    _children.push_back(child_);
    child_->SetParent(this);
}

void GFile::RemoveChild(GFile * child_, bool bRemoveAllChildren)
{
    // todo(gzy): impl remove all children.
    auto it = std::find(_children.begin(), _children.end(), child_);
    if (it != _children.end())
    {
        _children.erase(it);
    }
}

bool GFile::IsSamePath(GFile * file_)
{
    return _FullPathUUID == file_->FullPathUUID();
}

bool GFile::IsDifferent(GFile * file_)
{
    bool ret = false;
    if (file_->Stat().st_size != _stat.st_size)
    {
        ret = true;
    }
    else if (file_->Stat().st_mtimespec.tv_sec != _stat.st_mtimespec.tv_sec)
    {
        if (memcmp(file_->Sha(), Sha(), SHA_CHAR_LENGTH) != 0)
        {
            ret = true;
        }
        else
        {
            _stat = file_->Stat();
        }
    }
    return ret;
}

bool GFile::IsChild(GFile * file_)
{
    bool ret = false;
    
    
    if (strlen(file_->FullPath()) > strlen(_fullPath) &&
        strncmp(_fullPath, file_->FullPath(), strlen(_fullPath)) == 0)
    {
        ret = true;
    }
    
    return ret;
}

bool GFile::IsFolder()
{
    return S_ISDIR(_stat.st_mode);
}

void GFile::SortChildren()
{
    std::sort(_children.begin(), _children.end(), [](GFile * & x, GFile * & y){
         if (x->IsFolder() && !y->IsFolder()) return true;
         else if (!x->IsFolder() && y->IsFolder()) return false;
         else return x->_id < y->_id;
     });
}

std::string GFile::ToString(bool bVerbose)
{
    SaveSize();
    
    char tempChar[300];
    if (IsFolder())
    {
        sprintf(tempChar, CONSOLE_COLOR_FOLDER "%s/" CONSOLE_COLOR_END, _name);
    }
    else
    {
        char sizeBuf[FORMAT_FILESIZE_BUFFER_LENGTH];
        GTools::FormatFileSize(_stat.st_size, sizeBuf);
        
        if (bVerbose)
        {
            sprintf(tempChar, "%s (%s), %s", _name, sizeBuf, GTools::FormatShaToHex(Sha()).c_str());
        }
        else
        {
            sprintf(tempChar, "%s (%s)", _name, sizeBuf);
        }
    }
    
    return tempChar;
}

size_t GFile::ToBin(char * data_, size_t offset_)
{
    size_t ret = SaveSize();
    char * cur = data_ + offset_;
    
    // size
    memcpy(cur, (char *)&ret, DB_OFFSET_LENGTH);
    cur += DB_OFFSET_LENGTH;
    
    // id
    memcpy(cur, (char *)&_id, sizeof(long));
    cur += sizeof(long);
    
    // parent_id
    memcpy(cur, (char *)&_parent->_id, sizeof(long));
    cur += sizeof(long);
    
    // stat
    memcpy(cur, (char *)&_stat, sizeof(struct stat));
    cur += sizeof(struct stat);
    
    // name
    memcpy(cur, _name, strlen(_name) + 1);
    cur += strlen(_name) + 1;
    
    // sha
    memcpy(cur, Sha(), SHA_CHAR_LENGTH);
    
    return ret + DB_OFFSET_LENGTH;
}

size_t GFile::SaveSize()
{
    size_t nameLength = 0;
    if (_name != nullptr)
    {
        nameLength = strlen(_name);
    }
    // id, parent_id, stat, name, sha;
    size_t ret = sizeof(long) + sizeof(long) + sizeof(struct stat) + nameLength + 1 + SHA_CHAR_LENGTH;
    
    return ret;
}


void GFile::ReGenerateID()
{
    _id = ++_id_automatic_inc;
}
