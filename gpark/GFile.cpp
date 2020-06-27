
#include <fstream>
#include <dirent.h>
#include <openssl/sha.h>

#include "GTools.h"
#include "GPark.h"

#include "GFile.h"

long GFile::_id_automatic_inc = 0;

GFile::GFile(char * data_, size_t size_, long & parent_id_)
    : _id(0)
    , _parent(nullptr)
    , _name("")
    , _bGenShaed(false)
{
    size_t nameSize = size_ - SaveSize() - DB_OFFSET_LENGTH;
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
    
    memcpy(buffer, data_, nameSize + 1);
    data_ += nameSize + 1;
    _name = buffer;
    
    memcpy(buffer, data_, SHA_CHAR_LENGTH);
    data_ += SHA_CHAR_LENGTH;
    memcpy(Sha(), buffer, SHA_CHAR_LENGTH);
    
    delete [] buffer;
}

GFile::GFile(GFile * parent_, const std::string & fullPath_, struct dirent * dirent_, long id_)
    : _id(id_)
    , _parent(parent_)
    , _fullPath(fullPath_)
    , _name("")
    , _bGenShaed(false)
{
    memset(_sha, 0, SHA_CHAR_LENGTH);
    
    if (_id == -1)
    {
        _id = ++_id_automatic_inc;
    }
    
    if (dirent_)
    {
        _name = dirent_->d_name;
    }
    stat(_fullPath.c_str(), &_stat);
}

GFile::~GFile()
{
    
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
    if (GPark::Instance()->GetWorkPath().size() >= FullPath().size())
    {
        return "./";
    }
    else
    {
        return FullPath().c_str() + GPark::Instance()->GetWorkPath().size() + 1;
    }
}
const std::string & GFile::FullPath()
{
    if (_fullPath.empty() && _parent != nullptr)
    {
        _fullPath = _parent->FullPath() + "/" + _name;
    }
    return _fullPath;
}
const std::string & GFile::Name()
{
    return _name;
}
unsigned char * GFile::Sha()
{
    if (!_bGenShaed)
    {
        _bGenShaed = true;
        
        if (!Name().empty() && !FullPath().empty() && !IsFolder())
        {
            SHA_CTX ctx;
            std::ifstream ifile;
            ifile.open(FullPath().c_str(), std::ios::in | std::ios::binary);
            char * buffer = new char[_stat.st_size];
            ifile.read(buffer, _stat.st_size);
            
            SHA1_Init(&ctx);
            SHA1_Update(&ctx, buffer, _stat.st_size);
            SHA1_Final(_sha, &ctx);
            
            ifile.close();
            delete[] buffer;
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
    _parent = file_->Parent();
    _fullPath = file_->FullPath();
    _name = file_->Name();
    _bGenShaed = file_->_bGenShaed;
    memcpy(_sha, file_->Sha(), SHA_CHAR_LENGTH);
    _stat = file_->Stat();
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

bool GFile::IsDifferent(GFile * file_)
{
    bool ret = false;
    if (file_->Stat().st_size != _stat.st_size ||
        (file_->Stat().st_mtimespec.tv_nsec != _stat.st_mtimespec.tv_nsec &&
         memcmp(file_->Sha(), Sha(), SHA_CHAR_LENGTH) != 0))
    {
        return true;
    }
    return ret;
}

bool GFile::IsChild(GFile * file_)
{
    bool ret = false;
    
    if (file_->FullPath().size() > FullPath().size() &&
        strncmp(FullPath().c_str(), file_->FullPath().c_str(), FullPath().size()) == 0)
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

std::string GFile::ToString()
{
    SaveSize();
    
    char tempChar[300];
    std::string ret;
    if (IsFolder())
    {
        sprintf(tempChar, CONSOLE_COLOR_FOLDER "%s/" CONSOLE_COLOR_END, _name.c_str());
        ret = tempChar;
    }
    else
    {
        char sizeBuf[50];
        GTools::FormatFileSize(_stat.st_size, sizeBuf);
        sprintf(tempChar, "%s (%s)", _name.c_str(), sizeBuf);
        
        ret = tempChar;
//        for (int i = 0; i < SHA_CHAR_LENGTH; i++) {
//            sprintf(tempChar, "%02x", _sha[i]);
//            ret += tempChar;
//        }
//        ret.push_back(')');
    }
    
    return ret;
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
    memcpy(cur, _name.c_str(), _name.size() + 1);
    cur += _name.size() + 1;
    
    // sha
    memcpy(cur, Sha(), SHA_CHAR_LENGTH);
    
    return ret + DB_OFFSET_LENGTH;
}

size_t GFile::SaveSize()
{
    // id, parent_id, stat, name, sha;
    size_t ret = sizeof(long) + sizeof(long) + sizeof(struct stat) + _name.size() + 1 + SHA_CHAR_LENGTH;
    
    return ret;
}
