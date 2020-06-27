
#include <fstream>
#include <dirent.h>
#include <openssl/sha.h>

#include "GFile.h"

long GFile::_id_automatic_inc = 0;

GFile::GFile(char * data_, size_t size_, long & parent_id_)
    : _id(0)
    , _parent(nullptr)
    , _name("")
{
    size_t nameSize = size_ - Size() - DB_OFFSET_LENGTH;
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
    memcpy(_sha, buffer, SHA_CHAR_LENGTH);
    
    delete [] buffer;
}

GFile::GFile(GFile * parent_, const std::string & fullPath_, struct dirent * dirent_, long id_)
    : _id(id_)
    , _parent(parent_)
    , _fullPath(fullPath_)
    , _name("")
{
    memset(_sha, 0, SHA_CHAR_LENGTH);
    
    if (_id == -1)
    {
        _id = ++_id_automatic_inc;
    }
    
    if (dirent_)
    {
        _name = dirent_->d_name;
        stat(_fullPath.c_str(), &_stat);
        
        CalSha();
    }
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
const std::string & GFile::FullPath()
{
    return _fullPath;
}
const std::string & GFile::Name()
{
    return _name;
}
unsigned char * GFile::Sha()
{
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

void GFile::AppendChildren(GFile * child_)
{
    _children.push_back(child_);
    child_->SetParent(this);
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
    Size();
    
    char tempChar[300];
    std::string ret;
    if (IsFolder())
    {
        sprintf(tempChar, CONSOLE_COLOR_FONT_BLUE "%s (%ld)" CONSOLE_COLOR_END "/", _name.c_str(), _id);
        ret = tempChar;
    }
    else
    {
        sprintf(tempChar, "%s (%ld), %lld, %ld (", _name.c_str(), _id, _stat.st_size, _stat.st_mtimespec.tv_sec);
        
        ret = tempChar;
        for (int i = 0; i < SHA_CHAR_LENGTH; i++) {
            sprintf(tempChar, "%02x", _sha[i]);
            ret += tempChar;
        }
        ret.push_back(')');
    }
    
    return ret;
}

size_t GFile::ToBin(char * data_, size_t offset_)
{
    size_t ret = Size();
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
    memcpy(cur, _sha, SHA_CHAR_LENGTH);
    
    return ret + DB_OFFSET_LENGTH;
}

void GFile::CalSha()
{
    SHA_CTX ctx;
    std::ifstream ifile;
    ifile.open(_fullPath.c_str(), std::ios::in | std::ios::binary);
    char * buffer = new char[_stat.st_size];
    ifile.read(buffer, _stat.st_size);
    
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, buffer, _stat.st_size);
    SHA1_Final(_sha, &ctx);
    
    ifile.close();
    delete[] buffer;
}

size_t GFile::Size()
{
    // id, parent_id, stat, name, sha;
    size_t ret = sizeof(long) + sizeof(long) + sizeof(struct stat) + _name.size() + 1 + SHA_CHAR_LENGTH;
    
    return ret;
}
