
#include <map>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "GPark.h"

#include "GFileMgr.h"
#include "GFile.h"

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
    
    if (access((_WorkPath + "/" REPOS_PATH_DB).c_str(), F_OK) != -1)
    {
        std::cout << "Reinitialized existing GPark repository." << std::endl;
    }
    else
    {
        mkdir((_WorkPath + "/" REPOS_PATH_FOLDER).c_str(), S_IRWXU);
        
        std::ofstream ofile;
        ofile.open((_WorkPath + "/" REPOS_PATH_INDEX).c_str(), std::ios::out);
        ofile << "gpark 0.01" << std::endl;

        ofile.close();
        
        _HomePath = _WorkPath;
        
        _savedRoot = GFileMgr::LoadFromPath(_HomePath.c_str());

        SaveDB(_savedRoot);
        
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

bool GPark::DetectGParkPath()
{
    bool ret = true;
    
    std::string currentStr = _WorkPath;
    
    while (access((currentStr + "/" REPOS_PATH_DB).c_str(), F_OK) == -1)
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

void GPark::Stats()
{
    if (!_HomePath.empty())
    {
        LoadDB();
        
        GFile * nowFileRoot = GFileMgr::LoadFromPath(_WorkPath.c_str());
        GFile * savedFileRoot = GFileMgr::GetFile(_savedRoot, nowFileRoot->FullPathUUID());
        
        std::vector<GFile *> changesList, missList, addList, detailAddList;
        
        GFileMgr::DifferentFileList(savedFileRoot, nowFileRoot, changesList, missList, addList, detailAddList);
        
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
    else
    {
        std::cout << "fatal: not a gpark repository." << std::endl;
    }
}

void GPark::Tree()
{
    std::string treeStr;
    
    GFileMgr::Tree(GFileMgr::LoadFromPath(_WorkPath.c_str()), &treeStr, false);
    
    std::cout << treeStr << std::endl;
}

void GPark::Show(bool bVerbose)
{
    if (!_HomePath.empty())
    {
        LoadDB();
        
        std::string treeStr;
        
        GFileMgr::Tree(_savedRoot, &treeStr, bVerbose);
        
        std::cout << treeStr << std::endl;
    }
    else
    {
        std::cout << "fatal: not a gpark repository." << std::endl;
    }
}

void GPark::Save()
{
    if (!_HomePath.empty())
    {
        LoadDB();
        
        std::vector<GFile *> changesList, missList, addList, detailAddList;
        
        GFile * nowFileRoot = GFileMgr::LoadFromPath(_WorkPath.c_str());
        GFile * savedFileRoot = GFileMgr::GetFile(_savedRoot, nowFileRoot->FullPathUUID());
        
        GFileMgr::DifferentFileList(savedFileRoot, nowFileRoot, changesList, missList, addList, detailAddList);
        
        GFile * cur = nullptr;
        for (int i = 0; i < missList.size(); ++i)
        {
            cur = GFileMgr::GetFile(savedFileRoot, missList[i]->FullPathUUID());
            GAssert(cur != nullptr, "save error. can't find %s", missList[i]->FullPath());
            
            cur->Parent()->RemoveChild(cur);
        }
        
        for (int i = 0; i < changesList.size(); ++i)
        {
            cur = GFileMgr::GetFile(savedFileRoot, changesList[i]->FullPathUUID());
            GAssert(cur != nullptr, "save error. can't find %s", changesList[i]->FullPath());
            
            cur->CopyFrom(changesList[i]);
        }
        
        for (int i = 0; i < detailAddList.size(); ++i)
        {
            cur = GFileMgr::GetFile(savedFileRoot, detailAddList[i]->Parent()->FullPathUUID());
            GAssert(cur != nullptr, "save error. can't find %s", detailAddList[i]->Parent()->FullPath());
            
            cur->AppendChild(detailAddList[i]);
        }
        cur->SortChildren();
        
        SaveDB(_savedRoot);
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

void GPark::LoadDB()
{
    std::ifstream ifile;
    
    ifile.open((_HomePath + "/" REPOS_PATH_DB).c_str(), std::ios::in | std::ios::binary);
    
    char * readBuffer = new char[DB_OFFSET_LENGTH];
    
    ifile.read(readBuffer, DB_OFFSET_LENGTH);
    
    size_t size = *((size_t*)readBuffer);
    delete[] readBuffer;
    
    readBuffer = new char[size - DB_OFFSET_LENGTH];
    ifile.read(readBuffer, size - DB_OFFSET_LENGTH);
    _savedRoot = GFileMgr::Load(readBuffer, size - DB_OFFSET_LENGTH);
    
    ifile.close();
    delete [] readBuffer;
}

void GPark::SaveDB(GFile * root_)
{
    std::ofstream ofile;
    ofile.open((_HomePath + "/" REPOS_PATH_DB).c_str(), std::ios::out | std::ios::binary);
    
    size_t size = 0, offset = DB_OFFSET_LENGTH;
    GFileMgr::CheckSize(root_, size);
    
    size += DB_OFFSET_LENGTH;
    
    char * writeBuffer = new char[size];
    
    memcpy(writeBuffer, (char *)&size, DB_OFFSET_LENGTH);
    
    GFileMgr::ToBin(root_, writeBuffer, offset);
    
    ofile.write(writeBuffer, size);
    
    ofile.close();
    delete [] writeBuffer;
}

GPark::GPark()
    : _WorkPath("")
    , _HomePath("")
    , _savedRoot(nullptr)
{
    char tmpChar[1024];
    getcwd(tmpChar, 1024);
    _WorkPath = tmpChar;
    
    DetectGParkPath();
}

GPark::~GPark()
{

}
