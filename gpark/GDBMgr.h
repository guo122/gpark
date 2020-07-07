
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
    static GFileTree * LoadDB(const char * dbHomePath_, const char * globalHomePath_);
    static void SaveDB(const char * globalHomePath_, GFileTree * fileTree_, unsigned threadNum_);
    
public:
    static char CheckDBVersion(char * dbBuffer_);
    
private:
    static GFileTree * LoadDB_Impl(const char * globalHomePath_, char * dbBuffer_, struct stat & dbStat_);
    static void SaveDB_Impl(const char * globalHomePath_, GFileTree * fileTree_, unsigned threadNum_);
    
private:
    GDBMgr();
    ~GDBMgr();
};

#endif /* _GDBMGR_H_ */
