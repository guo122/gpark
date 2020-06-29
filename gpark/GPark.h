
#ifndef _GPARK_H_
#define _GPARK_H_

#include "Defines.h"

class GFile;
class GFileTree;

class GPark
{
public:
    static GPark * Instance();
    
public:
    void InitDB();
    void Status(bool bMissignore);
    void Tree(int depth);
    void Show(bool bVerbose, int depth);
    void Save();
    void Diff(const char * otherRepos_);
    
    void Destory();
    
public:
    std::string GetWorkPath();
    std::string GetHomePath();
    
private:
    void DiffRepos(bool bMissignore, GFileTree * thisRepos, GFileTree * otherRepos);
    bool DetectGParkPath();
    GFileTree * LoadDB(const char * DBPath_);
    void SaveDB();
    
private:
    std::string _WorkPath;
    std::string _HomePath;
    
    GFileTree * _savedFileTree;
    
private:
    GPark();
    ~GPark();
    
    static GPark * _instance;
};

#endif /* _GPARK_H_ */
