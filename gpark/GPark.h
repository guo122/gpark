
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
    void InitDB(unsigned threadNum_);
    void Status(bool bMissignore, unsigned int threadNum_);
    void Tree(int depth, unsigned int threadNum_);
    void Show(bool bVerbose, int depth);
    void Save(unsigned threadNum_);
    void Diff(const char * otherRepos_);
    
    void Destory();
    
public:
    const char * GetGlobalWorkPath();
    const char * GetGlobalHomePath();
    
private:
    void DiffRepos(bool bMissignore, GFileTree * thisRepos, GFileTree * otherRepos);
    void DetectGParkPath();
    
private:
    const char * _GlobalWorkPath;
    const char * _GlobalHomePath;
    
    GFileTree * _savedFileTree;
    
private:
    GPark();
    ~GPark();
    
    static GPark * _instance;
};

#endif /* _GPARK_H_ */
