
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "GPark.h"

#include "GFileMgr.h"

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
    if (access((_WorkPath + REPOS_PATH_INDEX).c_str(), F_OK) != -1)
    {
        std::cout << "Reinitialized existing GPark repository." << std::endl;
    }
    else
    {
        mkdir((_WorkPath + REPOS_PATH_FOLDER).c_str(), S_IRWXU);
        
        std::ofstream ofile;
        ofile.open((_WorkPath + REPOS_PATH_INDEX).c_str(), std::ios::out);
        ofile << "gpark 0.01" << std::endl;
        
        ofile.close();
        
        _GParkPath = _WorkPath;
        
        GFile * rootFile = GFileMgr::Load();
             
        std::string treeStr;
        
        GFileMgr::Tree(rootFile, &treeStr);
        
        std::cout << treeStr << std::endl;
        
        SaveDB(rootFile);
    }
}

std::string GPark::GetWorkPath()
{
    return _WorkPath;
}

std::string GPark::GetGparkPath()
{
    return _GParkPath;
}

bool GPark::DetectGParkPath()
{
    bool ret = true;
    
    std::string currentStr = _WorkPath;
    
    while (access((currentStr + REPOS_PATH_INDEX).c_str(), F_OK) == -1)
    {
        auto last_index = currentStr.find_last_of("/");
        currentStr.erase(currentStr.begin() + last_index, currentStr.end());
        if (currentStr.empty())
        {
            ret = false;
            break;
        }
    }
     
    _GParkPath = currentStr;
    
    return ret;
}

void GPark::Stats()
{
    if (!_GParkPath.empty())
    {
        LoadDB();
        
        // todo(gzy): now
        std::string treeStr;
        
        GFileMgr::Tree(_savedRoot, &treeStr);
        
        std::cout << treeStr << std::endl;
    }
    else
    {
        std::cout << "fatal: not a gpark repository." << std::endl;
    }
}

void GPark::Tree()
{
    std::string treeStr;
    
    GFileMgr::Tree(GFileMgr::Load(), &treeStr);
    
    std::cout << treeStr << std::endl;
}

void GPark::SaveDB(GFile * root)
{
    std::ofstream ofile;
    ofile.open((_GParkPath + REPOS_PATH_DB).c_str(), std::ios::out | std::ios::binary);
    
    size_t size = 0, offset = DB_OFFSET_LENGTH;
    GFileMgr::CheckSize(root, size);
    
    size += DB_OFFSET_LENGTH;
    
    char * writeBuffer = new char[size];
    
    memcpy(writeBuffer, (char *)&size, DB_OFFSET_LENGTH);
    
    GFileMgr::ToBin(root, writeBuffer, offset);
    
    ofile.write(writeBuffer, size);
    
    ofile.close();
    delete [] writeBuffer;
}

void GPark::LoadDB()
{
    std::ifstream ifile;
    
    ifile.open((_GParkPath + REPOS_PATH_DB).c_str(), std::ios::in | std::ios::binary);
    
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

GPark::GPark()
    : _WorkPath("")
    , _GParkPath("")
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
