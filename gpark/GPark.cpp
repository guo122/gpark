
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
    
    if (access((_WorkPath + "/" GPARK_PATH_DB).c_str(), F_OK) != -1)
    {
        std::cout << "Reinitialized existing GPark repository." << std::endl;
    }
    else
    {
        mkdir((_WorkPath + "/" GPARK_PATH_HOME).c_str(), S_IRWXU);
        
        std::ofstream ofile;
        ofile.open((_WorkPath + "/" GPARK_PATH_INDEX).c_str(), std::ios::out);
        ofile << "gpark 0.01" << std::endl;

        ofile.close();
        
        _HomePath = _WorkPath;
        
        _savedFileTree = GFileMgr::LoadFromPath(_HomePath.c_str());

        SaveDB();
        
        std::cout << "init db done. " << std::endl;
    }
}

std::string GPark::GetWorkPath()
{
    return _WorkPath;
}

std::string GPark::GetHomePath()
{
    return _HomePath;
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
    
    std::string currentStr = _WorkPath;
    
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
     
    _HomePath = currentStr;
    
    return ret;
}

void GPark::Status(bool bMissignore)
{
    _savedFileTree = LoadDB((_HomePath + "/" GPARK_PATH_DB).c_str());
    if (_savedFileTree)
    {
        GFileTree * nowFileTree = GFileMgr::LoadFromPath(_WorkPath.c_str());
        GFileTree * thisFileTree = _savedFileTree->GetSubTree(nowFileTree->Root()->FullPathUUID());
        
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
    
    GFileMgr::Tree(GFileMgr::LoadFromPath(_WorkPath.c_str())->Root(), &treeStr, false, depth);
    
    std::cout << treeStr << std::endl;
}

void GPark::Show(bool bVerbose, int depth)
{
    _savedFileTree = LoadDB((_HomePath + "/" GPARK_PATH_DB).c_str());
    if (_savedFileTree)
    {
        std::string treeStr;
        
        GFileMgr::Tree(_savedFileTree->GetFile(GFileMgr::GetUUID(_WorkPath)), &treeStr, bVerbose, depth);
        
        std::cout << treeStr << std::endl;
    }
    else
    {
        std::cout << "fatal: not a gpark repository." << std::endl;
    }
}

void GPark::Save()
{
    _savedFileTree = LoadDB((_HomePath + "/" GPARK_PATH_DB).c_str());
    if (_savedFileTree)
    {
        std::vector<GFile *> changesList, missList, addList;
        
        GFileTree * nowFileTree = GFileMgr::LoadFromPath(_WorkPath.c_str());
        GFileTree * thisFileTree = _savedFileTree->GetSubTree(nowFileTree->Root()->FullPathUUID());
        
        GFileMgr::DifferentFileList(true, thisFileTree, nowFileTree, changesList, missList, addList);
        
        GFile * cur = nullptr;
        for (int i = 0; i < missList.size(); ++i)
        {
            cur = _savedFileTree->GetFile(missList[i]->FullPathUUID());
            GAssert(cur != nullptr, "save error. can't find %s", missList[i]->FullPath().c_str());
            
            cur->Parent()->RemoveChild(cur);
        }
        
        for (int i = 0; i < changesList.size(); ++i)
        {
            cur = _savedFileTree->GetFile(changesList[i]->FullPathUUID());
            GAssert(cur != nullptr, "save error. can't find %s", changesList[i]->FullPath().c_str());
            
            cur->CopyFrom(changesList[i]);
        }
        
        for (int i = 0; i < addList.size(); ++i)
        {
            cur = _savedFileTree->GetFile(addList[i]->Parent()->FullPathUUID());
            GAssert(cur != nullptr, "save error. can't find %s", addList[i]->Parent()->FullPath().c_str());
            
            cur->AppendChild(addList[i]);
            _savedFileTree->Refresh(false);
        }
        
        cur->SortChildren();
        
        SaveDB();
    }
    else
    {
        std::cout << "fatal: not a gpark repository." << std::endl;
    }
}

void GPark::Diff(const char * otherRepos_)
{
    _savedFileTree = LoadDB((_HomePath + "/" GPARK_PATH_DB).c_str());
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
        unsigned char dbSha[SHA_CHAR_LENGTH];
        struct stat dbStat;
        stat(DBPath_, &dbStat);
        
        char * readBuffer = new char[dbStat.st_size];
        ifile.read(readBuffer, dbStat.st_size);
        
        SHA_CTX ctx;
        SHA1_Init(&ctx);
        SHA1_Update(&ctx, readBuffer, dbStat.st_size);
        SHA1_Final(dbSha, &ctx);
        std::cout << "loading...DB(" CONSOLE_COLOR_FONT_CYAN << GTools::FormatShaToHex(dbSha) << CONSOLE_COLOR_END ")" CONSOLE_COLOR_FONT_YELLOW << GTools::FormatTimestampToYYMMDD_HHMMSS(dbStat.st_mtimespec.tv_sec) << CONSOLE_COLOR_END << std::endl;
   
        ret = GFileMgr::LoadFromDB(readBuffer + DB_OFFSET_LENGTH, dbStat.st_size - DB_OFFSET_LENGTH);
        
        ifile.close();
        delete [] readBuffer;
        
    }
    
    return ret;
}

void GPark::SaveDB()
{
    std::ofstream ofile;
    ofile.open((_HomePath + "/" GPARK_PATH_DB).c_str(), std::ios::out | std::ios::binary);
    
    _savedFileTree->Refresh(true);
    size_t size = _savedFileTree->CheckSize() + DB_OFFSET_LENGTH;

    char * writeBuffer = new char[size];
    
    memcpy(writeBuffer, (char *)&size, DB_OFFSET_LENGTH);
    
    _savedFileTree->ToBin(writeBuffer + DB_OFFSET_LENGTH);
    
    ofile.write(writeBuffer, size);
    
    ofile.close();
    delete [] writeBuffer;
}

GPark::GPark()
    : _WorkPath("")
    , _HomePath("")
    , _savedFileTree(nullptr)
{
    char tmpChar[1024];
    getcwd(tmpChar, 1024);
    _WorkPath = tmpChar;
    
    if (DetectGParkPath())
    {
        GFileMgr::LoadIgnoreFile(_HomePath);
        GFileMgr::LoadMissIgnoreFile(_HomePath);
    }
}

GPark::~GPark()
{

}
