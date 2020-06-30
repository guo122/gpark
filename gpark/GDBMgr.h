
#ifndef _GDBMGR_H_
#define _GDBMGR_H_

#include <sys/stat.h>

#include "Defines.h"

class GFileTree;

class GDBMgr
{
    /* DB_VERSION v1
     |---1---|,|-----20------|,|-------8-------|----------------|..........
     |version|,|This save Sha|,|The File offset|The File content|...loop...
     */
public:
    static GFileTree * LoadDB(const char * globalHomePath_);
    static void SaveDB(const char * globalHomePath_, GFileTree * fileTree_);
    
public:
    static char CheckDBVersion(char * dbBuffer_);
    static void VersionConvert(char oriDBVerion,
                               char * oriDBBuffer_,
                               size_t oriSize_,
                               char * dstDBBuffer_,
                               char dstDBVersion,
                               size_t & dstSize_);
    
private:
    static GFileTree * LoadDBV1(const char * globalHomePath_, char * dbBuffer_, struct stat & dbStat_);
    
private:
    static void SaveDBV1(const char * globalHomePath_, GFileTree * fileTree_);
    
private:
    GDBMgr();
    ~GDBMgr();
};

#endif /* _GDBMGR_H_ */
