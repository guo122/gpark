
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
    
public:
    std::string GetWorkPath();
    std::string GetGparkPath();
    
private:
    bool DetectGParkPath();
    void SaveDB(GFile * root);
    void LoadDB();
    
private:
    std::string _WorkPath;
    std::string _GParkPath;
    
    GFile * _savedRoot;
    
private:
    GPark();
    ~GPark();
    
    static GPark * _instance;
};

#endif /* _GPARK_H_ */
