
#include <fstream>

#include "GThreadHelper.h"

#include "GTools.h"
#include "GFileTree.h"
#include "GFile.h"

#include "GDBMgr.h"

GFileTree * GDBMgr::LoadDB(const char * globalHomePath_)
{
    GFileTree * ret = nullptr;
    
    std::ifstream ifile;
    std::string homePathStr = globalHomePath_;
    ifile.open((homePathStr + "/" GPARK_PATH_DB).c_str(), std::ios::in | std::ios::binary);
    
    if (ifile.is_open())
    {
        struct stat dbStat;
        stat((homePathStr + "/" GPARK_PATH_DB).c_str(), &dbStat);
        
        if (dbStat.st_size > 0)
        {
            char * readBuffer = new char[dbStat.st_size];
            ifile.read(readBuffer, dbStat.st_size);
            
            char dbVersion = CheckDBVersion(readBuffer);
            if (dbVersion == DB_VERSION)
            {
                ret = LoadDBV1(globalHomePath_, readBuffer, dbStat);
            }
            else
            {
                // todo(gzy): log....
                std::cout << CONSOLE_COLOR_FONT_RED "fatal" CONSOLE_COLOR_END ": this db version not support." << std::endl;
            }
            
            delete [] readBuffer;
        }
        
        ifile.close();
    }
    
    return ret;
}

void GDBMgr::SaveDB(const char * globalHomePath_, GFileTree * fileTree_, unsigned threadNum_)
{
#if DB_VERSION == DB_VERSION_V1
    SaveDBV1(globalHomePath_, fileTree_, threadNum_);
#else
    GAssert(false, "this db version not support.";)
#endif
}

char GDBMgr::CheckDBVersion(char * dbBuffer_)
{
    char ret;
    memcpy(&ret, dbBuffer_, 1);
    return ret;
}

void GDBMgr::VersionConvert(char oriDBVerion,
                              char * oriDBBuffer_,
                              size_t oriSize_,
                              char * dstDBBuffer_,
                              char dstDBVersion,
                              size_t & dstSize_)
{
}

GFileTree * GDBMgr::LoadDBV1(const char * globalHomePath_, char * dbBuffer_, struct stat & dbStat_)
{
    GFile * root = nullptr;
    GFile * parent = nullptr;
    GFile * cur = nullptr;
    long parent_id = -122;
    unsigned char dbSha[SHA_CHAR_LENGTH];
    
    std::map<long, GFile*> gfileMap;
    std::map<long, GFile*>::iterator it;
    
    size_t offset = 1 + SHA_CHAR_LENGTH, size = 0;
    
    memcpy(dbSha, dbBuffer_ + 1, SHA_CHAR_LENGTH);
    
    std::cout << "loading...DB(" CONSOLE_COLOR_FONT_CYAN << GTools::FormatShaToHex(dbSha) << CONSOLE_COLOR_END ")" CONSOLE_COLOR_FONT_YELLOW << GTools::FormatTimestampToYYMMDD_HHMMSS(dbStat_.st_mtimespec.tv_sec) << CONSOLE_COLOR_END << std::endl;
    
    char offsetFormatBuf[FORMAT_FILESIZE_BUFFER_LENGTH];
    char sizeFormatBuf[FORMAT_FILESIZE_BUFFER_LENGTH];
    char outputBuf[1024];
    bool * outputRunning = new bool;
    *outputRunning = true;
    GTools::FormatFileSize(dbStat_.st_size, sizeFormatBuf, CONSOLE_COLOR_FONT_CYAN);
    
    std::thread outputThread(GThreadHelper::PrintLog, outputBuf, outputRunning);
    
    while (offset < dbStat_.st_size)
    {
        GTools::FormatFileSize(offset, offsetFormatBuf, CONSOLE_COLOR_FONT_CYAN);
        sprintf(outputBuf, CONSOLE_CLEAR_LINE "\r(%s/%s)", offsetFormatBuf, sizeFormatBuf);
        
        size = *((size_t*)(dbBuffer_ + offset)) + DB_OFFSET_LENGTH;

        cur = new GFile(dbBuffer_ + offset, size, parent_id);
        if (root == nullptr)
        {
            root = new GFile(nullptr, globalHomePath_, nullptr, parent_id);
            gfileMap.insert(std::pair<long, GFile*>(parent_id, root));
        }
        
        parent = gfileMap[parent_id];
        
        GAssert(parent, "can't find parent id when load DB.");

        parent->AppendChild(cur);
        cur->GenFullPath();
        
        it = gfileMap.find(cur->Id());
        GAssert(it == gfileMap.end(), "same id %ld (%s)", cur->Id(), cur->GlobalFullPath());
        
        gfileMap.insert(std::pair<long, GFile*>(cur->Id(), cur));
        
        offset += size;
    }
    *outputRunning = false;
    outputThread.join();
    delete outputRunning;
    std::cout << CONSOLE_CLEAR_LINE "\r(" << sizeFormatBuf << ").." CONSOLE_COLOR_FONT_GREEN "done" CONSOLE_COLOR_END << std::endl;
    
    return new GFileTree(root);
}

void GDBMgr::SaveDBV1(const char * globalHomePath_, GFileTree * fileTree_, unsigned threadNum_)
{
    char dbVersion = 1;
    std::ofstream ofile;
    std::string homePathStr = globalHomePath_;

    fileTree_->Refresh(true);
    size_t size = fileTree_->CheckSize() + SHA_CHAR_LENGTH + 1;

    char * writeBuffer = new char[size];
    memcpy(writeBuffer, &dbVersion, 1);
    
    size_t totalShaBufferLength = (fileTree_->GetFileList().size() - 1) * SHA_CHAR_LENGTH;
    char * totalShaBuffer = new char [totalShaBufferLength];
    fileTree_->ToBin(writeBuffer + 1 + SHA_CHAR_LENGTH, totalShaBuffer, threadNum_);
    
    unsigned char dbSha[SHA_CHAR_LENGTH];
    GTools::CalculateSHA1(totalShaBuffer, totalShaBufferLength, dbSha);
    
    memcpy(writeBuffer + 1, dbSha, SHA_CHAR_LENGTH);
    
    ofile.open((homePathStr + "/" GPARK_PATH_DB).c_str(), std::ios::out | std::ios::binary);
    ofile.write(writeBuffer, size);
    ofile.close();
    
    delete [] writeBuffer;
    delete [] totalShaBuffer;
}

GDBMgr::GDBMgr()
{
    
}

GDBMgr::~GDBMgr()
{
    
}
