
#include <map>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <openssl/sha.h>

#include "GPark.h"

#include "GTools.h"
#include "GFileMgr.h"
#include "GFile.h"
#include "GFileTree.h"

GPark * GPark::_instance = nullptr;

GPark * GPark::Instance()
{
    if (!_instance)
    {
        _instance = new GPark;
    }
    return _instance;
}

void GPark::InitDB()
{
    std::cout << "init..." << std::endl;
    std::string workPathStr = _GlobalWorkPath;
    
    if (access((workPathStr + "/" GPARK_PATH_DB).c_str(), F_OK) != -1)
    {
        std::cout << "Reinitialized existing GPark repository." << std::endl;
    }
    else
    {
        mkdir((workPathStr + "/" GPARK_PATH_HOME).c_str(), S_IRWXU);
        
        std::ofstream ofile;
        ofile.open((workPathStr + "/" GPARK_PATH_INDEX).c_str(), std::ios::out);
        ofile << "gpark 0.01" << std::endl;

        ofile.close();
        
        _GlobalHomePath = _GlobalWorkPath;
        
        _savedFileTree = GFileMgr::LoadFromPath(_GlobalHomePath);

        SaveDB();
        
        std::cout << "init db done. " << std::endl;
    }
}

const char * GPark::GetGlobalWorkPath()
{
    return _GlobalWorkPath;
}

const char * GPark::GetGlobalHomePath()
{
    return _GlobalHomePath;
}

void GPark::DiffRepos(bool bMissignore, GFileTree * thisRepos, GFileTree * otherRepos)
{
    std::vector<GFile *> changesList, missList, addList;
    
    GFileMgr::DifferentFileList(bMissignore, thisRepos, otherRepos, changesList, missList, addList);
    
    for (int i = 0; i < changesList.size(); ++i)
    {
        std::cout << CONSOLE_COLOR_FONT_YELLOW "[change]" CONSOLE_COLOR_END << changesList[i]->CurrentPath() << std::endl;
    }
    
    for (int i = 0; i < missList.size(); ++i)
    {
        if (missList[i]->IsFolder())
        {
            std::cout << CONSOLE_COLOR_FONT_RED "[miss]" CONSOLE_COLOR_END CONSOLE_COLOR_FOLDER << missList[i]->CurrentPath() << "/" CONSOLE_COLOR_END << std::endl;
        }
        else
        {
            std::cout << CONSOLE_COLOR_FONT_RED "[miss]" CONSOLE_COLOR_END << missList[i]->CurrentPath() << std::endl;
        }
    }
    
    for (int i = 0; i < addList.size(); ++i)
    {
        if (addList[i]->IsFolder())
        {
            std::cout << CONSOLE_COLOR_FONT_GREEN "[add]" CONSOLE_COLOR_END CONSOLE_COLOR_FOLDER << addList[i]->CurrentPath() << "/" CONSOLE_COLOR_END << std::endl;
        }
        else
        {
            std::cout << CONSOLE_COLOR_FONT_GREEN "[add]" CONSOLE_COLOR_END << addList[i]->CurrentPath() << std::endl;
        }
    }
}

bool GPark::DetectGParkPath()
{
    bool ret = true;
    
    std::string currentStr = _GlobalWorkPath;
    
    while (access((currentStr + "/" GPARK_PATH_DB).c_str(), F_OK) == -1)
    {
        auto last_index = currentStr.find_last_of("/");
        currentStr.erase(currentStr.begin() + last_index, currentStr.end());
        if (currentStr.empty())
        {
            ret = false;
            break;
        }
    }
     
    _GlobalHomePath = GFileMgr::GetGlobalFullPath(currentStr.c_str());
    
    return ret;
}

void GPark::Status(bool bMissignore)
{
    std::string homePathStr = _GlobalHomePath;
    _savedFileTree = LoadDB((homePathStr + "/" GPARK_PATH_DB).c_str());
    if (_savedFileTree)
    {
        GFileTree * nowFileTree = GFileMgr::LoadFromPath(_GlobalWorkPath);
        GFileTree * thisFileTree = _savedFileTree->GetSubTree(nowFileTree->Root()->GlobalFullPath());
        
        DiffRepos(bMissignore, thisFileTree, nowFileTree);
    }
    else
    {
        std::cout << "fatal: not a gpark repository." << std::endl;
    }
}

void GPark::Tree(int depth)
{
    // todo(gzy): tree show miss ignore.
    std::string treeStr;
    
    GFileMgr::Tree(GFileMgr::LoadFromPath(_GlobalWorkPath)->Root(), &treeStr, false, depth);
    
    std::cout << treeStr << std::endl;
}

void GPark::Show(bool bVerbose, int depth)
{
    std::string homePathStr = _GlobalHomePath;
    _savedFileTree = LoadDB((homePathStr + "/" GPARK_PATH_DB).c_str());
    if (_savedFileTree)
    {
        std::string treeStr;
        
        GFileMgr::Tree(_savedFileTree->GetFile(_GlobalWorkPath), &treeStr, bVerbose, depth);
        
        std::cout << treeStr << std::endl;
    }
    else
    {
        std::cout << "fatal: not a gpark repository." << std::endl;
    }
}

void GPark::Save()
{
    std::string homePathStr = _GlobalHomePath;
    _savedFileTree = LoadDB((homePathStr + "/" GPARK_PATH_DB).c_str());
    if (_savedFileTree)
    {
        std::vector<GFile *> changesList, missList, addList;
        
        GFileTree * nowFileTree = GFileMgr::LoadFromPath(_GlobalWorkPath);
        GFileTree * thisFileTree = _savedFileTree->GetSubTree(nowFileTree->Root()->GlobalFullPath());
        
        GFileMgr::DifferentFileList(true, thisFileTree, nowFileTree, changesList, missList, addList);
        
        GFile * cur = nullptr;
        for (int i = 0; i < missList.size(); ++i)
        {
            cur = _savedFileTree->GetFile(missList[i]->GlobalFullPath());
            GAssert(cur != nullptr, "save error. can't find %s", missList[i]->GlobalFullPath());
            
            cur->Parent()->RemoveChild(cur);
        }
        
        for (int i = 0; i < changesList.size(); ++i)
        {
            cur = _savedFileTree->GetFile(changesList[i]->GlobalFullPath());
            GAssert(cur != nullptr, "save error. can't find %s", changesList[i]->GlobalFullPath());
            
            cur->CopyFrom(changesList[i]);
        }
        
        for (int i = 0; i < addList.size(); ++i)
        {
            cur = _savedFileTree->GetFile(addList[i]->Parent()->GlobalFullPath());
            GAssert(cur != nullptr, "save error. can't find %s", addList[i]->Parent()->GlobalFullPath());
            
            cur->AppendChild(addList[i]);
            cur->SortChildren();
            
            _savedFileTree->Refresh(false);
        }
        
        SaveDB();
    }
    else
    {
        std::cout << "fatal: not a gpark repository." << std::endl;
    }
}

void GPark::Diff(const char * otherRepos_)
{
    std::string homePathStr = _GlobalHomePath;
    _savedFileTree = LoadDB((homePathStr + "/" GPARK_PATH_DB).c_str());
    if (_savedFileTree)
     {
         std::string otherReposStr = otherRepos_;
         otherReposStr += "/" GPARK_PATH_DB;
         
         GFileTree * otherFileTree = LoadDB(otherReposStr.c_str());
         
         if (otherFileTree == nullptr)
         {
             std::cout << "fatal: not found other repository." << std::endl;
         }
         else
         {
             DiffRepos(false, _savedFileTree, otherFileTree);
         }
     }
     else
     {
         std::cout << "fatal: not a gpark repository." << std::endl;
     }
}

void GPark::Destory()
{
    // todo(gzy): delete GFile *
}

GFileTree * GPark::LoadDB(const char * DBPath_)
{
    GFileTree * ret = nullptr;
    
    std::ifstream ifile;
    
    ifile.open(DBPath_, std::ios::in | std::ios::binary);
    
    if (ifile.is_open())
    {
        struct stat dbStat;
        stat(DBPath_, &dbStat);
        
        if (dbStat.st_size >= DB_OFFSET_LENGTH)
        {
            unsigned char dbSha[SHA_CHAR_LENGTH];
            
            char * readBuffer = new char[dbStat.st_size];
            ifile.read(readBuffer, dbStat.st_size);
            
            SHA_CTX ctx;
            SHA1_Init(&ctx);
            SHA1_Update(&ctx, readBuffer, dbStat.st_size);
            SHA1_Final(dbSha, &ctx);
            std::cout << "loading...DB(" CONSOLE_COLOR_FONT_CYAN << GTools::FormatShaToHex(dbSha) << CONSOLE_COLOR_END ")" CONSOLE_COLOR_FONT_YELLOW << GTools::FormatTimestampToYYMMDD_HHMMSS(dbStat.st_mtimespec.tv_sec) << CONSOLE_COLOR_END << std::endl;
            
            ret = GFileMgr::LoadFromDB(readBuffer + DB_OFFSET_LENGTH, dbStat.st_size - DB_OFFSET_LENGTH);
            
            delete [] readBuffer;
        }
        
        ifile.close();
    }
    
    return ret;
}

void GPark::SaveDB()
{
    std::ofstream ofile;
    std::string homePathStr = _GlobalHomePath;

    _savedFileTree->Refresh(true);
    size_t size = _savedFileTree->CheckSize() + DB_OFFSET_LENGTH;

    char * writeBuffer = new char[size];
    
    memcpy(writeBuffer, (char *)&size, DB_OFFSET_LENGTH);
    
    _savedFileTree->ToBin(writeBuffer + DB_OFFSET_LENGTH);
    
    ofile.open((homePathStr + "/" GPARK_PATH_DB).c_str(), std::ios::out | std::ios::binary);
    ofile.write(writeBuffer, size);
    ofile.close();
    
    delete [] writeBuffer;
}

GPark::GPark()
    : _GlobalWorkPath(nullptr)
    , _GlobalHomePath(nullptr)
    , _savedFileTree(nullptr)
{
    char workPathBuffer[FULLPATH_DEFAULT_BUFFER_LENGTH];
    getcwd(workPathBuffer, FULLPATH_DEFAULT_BUFFER_LENGTH);
    
    _GlobalWorkPath = GFileMgr::GetGlobalFullPath(workPathBuffer);

    if (DetectGParkPath())
    {
        GFileMgr::LoadIgnoreFile(_GlobalHomePath);
        GFileMgr::LoadMissIgnoreFile(_GlobalHomePath);
    }
}

GPark::~GPark()
{

}
