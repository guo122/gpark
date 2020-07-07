
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>

#include "GTools.h"
#include "GPark.h"
#include "GFileMgr.h"
#include "GFileTree.h"

#include "GFile.h"

long GFile::_id_automatic_inc = 0;

GFile::GFile()
    : _id(0)
    , _parent(nullptr)
    , _globalFullPath(nullptr)
    , _name(nullptr)
    , _bGenShaed(false)
    , _bFolder(false)
    , _fileSize(0)
    , _mTimestamp(0)
{
}

GFile::GFile(GFile * parent_, const char * globalFullPath_, struct dirent * dirent_, long id_)
    : _id(id_)
    , _parent(parent_)
    , _globalFullPath(const_cast<char *>(globalFullPath_))
    , _name(nullptr)
    , _bGenShaed(false)
    , _bFolder(false)
    , _fileSize(0)
    , _mTimestamp(0)
{
    memset(_sha, 0, SHA1_DIGEST_LENGTH);
    
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
    
    struct stat tempStat;
    stat(_globalFullPath, &tempStat);
    _bFolder = S_ISDIR(tempStat.st_mode);
    _fileSize = tempStat.st_size;
    _mTimestamp = tempStat.st_mtimespec.tv_sec;
}

GFile::~GFile()
{
    if (_name)
    {
        delete [] _name;
    }
}

size_t GFile::FromBin(char * data_, char * digestBuffer_, long * parent_id_)
{
    char * digestData = data_;
    char * buffer = new char[FILE_FILESIZE_LENGTH];
    
    // len, id, parent_id, isFolder, fileSize, mtime, sha1, nameSize, (name)
    memcpy(buffer, data_, FILE_FILESIZE_LENGTH);
    data_ += FILE_FILESIZE_LENGTH;
    size_t ret = *((size_t*)buffer);
    // id
    memcpy(buffer, data_, FILE_ID_LENGTH);
    data_ += FILE_ID_LENGTH;
    _id = *((long*)buffer);
    // parent_id
    memcpy(buffer, data_, FILE_ID_LENGTH);
    data_ += FILE_ID_LENGTH;
    *parent_id_ = *((long*)buffer);
    // isFolder
    memcpy(buffer, data_, FILE_BOOL_LENGTH);
    data_ += FILE_BOOL_LENGTH;
    _bFolder = *((bool*)buffer);
    // fileSize
    memcpy(buffer, data_, FILE_FILESIZE_LENGTH);
    data_ += FILE_FILESIZE_LENGTH;
    _fileSize = *((size_t*)buffer);
    // mTime
    memcpy(buffer, data_, FILE_MTIME_LENGTH);
    data_ += FILE_MTIME_LENGTH;
    _mTimestamp = *((long*)buffer);
    // sha1
    memcpy(_sha, data_, SHA1_DIGEST_LENGTH);
    data_ += SHA1_DIGEST_LENGTH;
    // nameSize
    memcpy(buffer, data_, FILE_NAMESIZE_LENGTH);
    data_ += FILE_NAMESIZE_LENGTH;
    int nameSize = *((int*)buffer);
    // name
    _name = new char[nameSize + 1];
    memcpy(_name, data_, nameSize);
    data_ += nameSize;
    _name[nameSize] = 0;
    
    delete [] buffer;
    _bGenShaed = true;
    
    // len,          // id, parent_id
    memcpy(digestBuffer_, digestData, FILE_FILESIZE_LENGTH);
    digestBuffer_ += FILE_FILESIZE_LENGTH;
    digestData += FILE_FILESIZE_LENGTH + FILE_ID_LENGTH + FILE_ID_LENGTH;
    // isFolder
    memcpy(digestBuffer_, digestData, FILE_BOOL_LENGTH);
    digestBuffer_ += FILE_BOOL_LENGTH;
    digestData += FILE_BOOL_LENGTH;
    // fileSize,        // mtime
    memcpy(digestBuffer_, digestData, FILE_FILESIZE_LENGTH);
    digestBuffer_ += FILE_FILESIZE_LENGTH;
    digestData += FILE_FILESIZE_LENGTH + FILE_MTIME_LENGTH;
    // sha1
    memcpy(digestBuffer_, digestData, SHA1_DIGEST_LENGTH);
    digestBuffer_ += SHA1_DIGEST_LENGTH;
    digestData += SHA1_DIGEST_LENGTH;
    // namesize
    memcpy(digestBuffer_, digestData, FILE_NAMESIZE_LENGTH);
    digestBuffer_ += FILE_NAMESIZE_LENGTH;
    digestData += FILE_NAMESIZE_LENGTH;
    // name
    memcpy(digestBuffer_, digestData, nameSize);
    digestBuffer_ += nameSize;
    digestData += nameSize;
    
    return ret;
}

size_t GFile::ToBin(char * buffer_, char * digestBuffer_)
{
    // len, id, parent_id, isFolder, fileSize, mtime, sha1, nameSize, (name)
    size_t ret = CheckBinLength();
    
    size_t nameSize = 0;
    if (_name != nullptr)
    {
        nameSize = strlen(_name);
    }
    
    // len
    memcpy(buffer_, (char *)&ret, FILE_FILESIZE_LENGTH);
    buffer_ += FILE_FILESIZE_LENGTH;
    // id
    memcpy(buffer_, (char *)&_id, FILE_ID_LENGTH);
    buffer_ += FILE_ID_LENGTH;
    // parent_id
    memcpy(buffer_, (char *)&(_parent->_id), FILE_ID_LENGTH);
    buffer_ += FILE_ID_LENGTH;
    // isFolder
    memcpy(buffer_, (char *)&(_bFolder), FILE_BOOL_LENGTH);
    buffer_ += FILE_BOOL_LENGTH;
    // fileSize
    memcpy(buffer_, (char *)&(_fileSize), FILE_FILESIZE_LENGTH);
    buffer_ += FILE_FILESIZE_LENGTH;
    // mtime
    memcpy(buffer_, (char *)&(_mTimestamp), FILE_MTIME_LENGTH);
    buffer_ += FILE_MTIME_LENGTH;
    // sha1
    memcpy(buffer_, Sha(), SHA1_DIGEST_LENGTH);
    buffer_ += SHA1_DIGEST_LENGTH;
    // namesize
    memcpy(buffer_, (char *)&nameSize, FILE_NAMESIZE_LENGTH);
    buffer_ += FILE_NAMESIZE_LENGTH;
    // name
    memcpy(buffer_, _name, nameSize);
    buffer_ += nameSize;
    
    // len
    memcpy(digestBuffer_, (char *)&ret, FILE_FILESIZE_LENGTH);
    digestBuffer_ += FILE_FILESIZE_LENGTH;
    // isFolder
    memcpy(digestBuffer_, (char *)&(_bFolder), FILE_BOOL_LENGTH);
    digestBuffer_ += FILE_BOOL_LENGTH;
    // fileSize
    memcpy(digestBuffer_, (char *)&(_fileSize), FILE_FILESIZE_LENGTH);
    digestBuffer_ += FILE_FILESIZE_LENGTH;
    // sha1
    memcpy(digestBuffer_, Sha(), SHA1_DIGEST_LENGTH);
    digestBuffer_ += SHA1_DIGEST_LENGTH;
    // namesize
    memcpy(digestBuffer_, (char *)&nameSize, FILE_NAMESIZE_LENGTH);
    digestBuffer_ += FILE_NAMESIZE_LENGTH;
    // name
    memcpy(digestBuffer_, _name, nameSize);
    digestBuffer_ += nameSize;
    
    return ret;
}

size_t GFile::CheckBinLength()
{
    size_t ret = 0;
    if (_name != nullptr)
    {
        ret = strlen(_name);
    }
    // len, id, parent_id, fileSize, mtime, sha1, nameSize, (name)
    ret += FILE_BASIC_LENGTH;
    
    return ret;
}

long GFile::Id()
{
    return _id;
}

GFile * GFile::Parent()
{
    return _parent;
}
const size_t & GFile::FileSize()
{
    return _fileSize;
}
const long & GFile::MTimestamp()
{
    return _mTimestamp;
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
    memcpy(_sha, file_->Sha(), SHA1_DIGEST_LENGTH);
    _bFolder = file_->IsFolder();
    _fileSize = file_->FileSize();
    _mTimestamp = file_->MTimestamp();
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
    if (file_->FileSize() != _fileSize)
    {
        ret = true;
    }
    else if (file_->MTimestamp() != _mTimestamp)
    {
        if (memcmp(file_->Sha(), Sha(), SHA1_DIGEST_LENGTH) != 0)
        {
            ret = true;
        }
        else
        {
            _mTimestamp = file_->MTimestamp();
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
    GTools::FormatFileSize(_fileSize, tempChar, CONSOLE_COLOR_FONT_CYAN);
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
        char * buffer = new char[_fileSize];
        ifile.read(buffer, _fileSize);
        ifile.close();

        GTools::CalculateSHA1(buffer, _fileSize, _sha);
        
        delete[] buffer;
    }
}

std::string GFile::ToString(bool bVerbose)
{
    CheckBinLength();
    
    char tempChar[300];
    if (IsFolder())
    {
        sprintf(tempChar, CONSOLE_COLOR_FOLDER "%s/" CONSOLE_COLOR_END, _name);
    }
    else
    {
        char sizeBuf[FORMAT_FILESIZE_BUFFER_LENGTH];
        GTools::FormatFileSize(_fileSize, sizeBuf);
        
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

void GFile::ReGenerateID()
{
    _id = ++_id_automatic_inc;
}
