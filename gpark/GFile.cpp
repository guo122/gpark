
#include <fstream>
#include <dirent.h>

#include "GTools.h"
#include "GPark.h"
#include "GFileMgr.h"
#include "GFileTree.h"

#include "GFile.h"

long GFile::_id_automatic_inc = 0;

GFile::GFile(char * data_, size_t size_, long & parent_id_)
    : _id(0)
    , _parent(nullptr)
    , _globalFullPath(nullptr)
    , _name(nullptr)
    , _bGenShaed(true)
    , _bFolder(false)
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
    _bFolder = S_ISDIR(_stat.st_mode);
    
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

GFile::GFile(GFile * parent_, const char * globalFullPath_, struct dirent * dirent_, long id_)
    : _id(id_)
    , _parent(parent_)
    , _globalFullPath(const_cast<char *>(globalFullPath_))
    , _name(nullptr)
    , _bGenShaed(false)
    , _bFolder(false)
{
    memset(_sha, 0, SHA_CHAR_LENGTH);
    
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
    stat(_globalFullPath, &_stat);
    _bFolder = S_ISDIR(_stat.st_mode);
}

GFile::~GFile()
{
    if (_name)
    {
        delete [] _name;
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
    if (strlen(GPark::Instance()->GetGlobalWorkPath()) >= strlen(_globalFullPath))
    {
        return "./";
    }
    else
    {
        return _globalFullPath + strlen(GPark::Instance()->GetGlobalWorkPath()) + 1;
    }
}
const char * GFile::GlobalFullPath()
{
    return _globalFullPath;
}
const char * GFile::Name()
{
    return _name;
}

unsigned char * GFile::Sha()
{
    CalSha();
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
    _bFolder = S_ISDIR(_stat.st_mode);
}

void GFile::GenFullPath()
{
    _globalFullPath = const_cast<char *>(GFileMgr::GetGlobalFullPath(_parent->GlobalFullPath(), _name));
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
    return _globalFullPath == file_->GlobalFullPath();
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
    
    
    if (strlen(file_->GlobalFullPath()) > strlen(_globalFullPath) &&
        strncmp(_globalFullPath, file_->GlobalFullPath(), strlen(_globalFullPath)) == 0)
    {
        ret = true;
    }
    
    return ret;
}

bool GFile::IsFolder()
{
    return _bFolder;
}

void GFile::SortChildren()
{
    std::sort(_children.begin(), _children.end(), [](GFile * & x, GFile * & y){
         if (x->IsFolder() && !y->IsFolder()) return true;
         else if (!x->IsFolder() && y->IsFolder()) return false;
         else return strcmp(x->Name(), y->Name()) < 0;
     });
}

bool GFile::IsNeedCalSha()
{
    bool ret = !_bGenShaed && _name != nullptr && !_bFolder;
    return ret;
}

void GFile::CalShaPreInfo(char * outputLog)
{
    char tempChar[FORMAT_FILESIZE_BUFFER_LENGTH];
    GTools::FormatFileSize(_stat.st_size, tempChar, CONSOLE_COLOR_FONT_CYAN);
    sprintf(outputLog, CONSOLE_CLEAR_LINE "\rcal sha(%s)", tempChar);
}

void GFile::CalSha()
{
    if (IsNeedCalSha())
    {
        _bGenShaed = true;
        
        GAssert(_globalFullPath, "has no full path.");
        
        std::ifstream ifile;
        ifile.open(_globalFullPath, std::ios::in | std::ios::binary);
        char * buffer = new char[_stat.st_size];
        ifile.read(buffer, _stat.st_size);
        ifile.close();

        GTools::CalculateSHA1(buffer, _stat.st_size, _sha);
        
        delete[] buffer;
    }
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
