
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
    void Stats();
    void Tree();
    void Show();
    void Save();
    
    void Destory();
    
public:
    std::string GetWorkPath();
    std::string GetHomePath();
    
private:
    bool DetectGParkPath();
    void LoadDB();
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
