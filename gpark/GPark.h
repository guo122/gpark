
#ifndef _GPARK_H_
#define _GPARK_H_

#include "Defines.h"

class GFile;

class GPark
{
public:
    static GPark * Instance();
    
public:
    void InitDB();
    void Status();
    void Tree();
    void Show(bool bVerbose);
    void Save();
    void Diff(const char * otherRepos_);
    
    void Destory();
    
public:
    std::string GetWorkPath();
    std::string GetHomePath();
    
private:
    void DiffRepos(GFile * thisRepos, GFile * otherRepos);
    bool DetectGParkPath();
    GFile * LoadDB(const char * DBPath_);
    void SaveDB(GFile * root_);
    
private:
    std::string _WorkPath;
    std::string _HomePath;
    
    GFile * _savedRoot;
    
private:
    GPark();
    ~GPark();
    
    static GPark * _instance;
};

#endif /* _GPARK_H_ */
