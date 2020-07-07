
#include <fstream>

#include "GThreadHelper.h"

#include "GTools.h"
#include "GFileTree.h"
#include "GFile.h"

#include "GDBMgr.h"

GFileTree * GDBMgr::LoadDB(const char * dbHomePath_, const char * globalHomePath_)
{
    GFileTree * ret = nullptr;
    
    std::ifstream ifile;
    std::string dbhomePathStr = dbHomePath_;
    ifile.open((dbhomePathStr + "/" GPARK_PATH_DB).c_str(), std::ios::in | std::ios::binary);
    
    if (ifile.is_open())
    {
        struct stat dbStat;
        stat((dbhomePathStr + "/" GPARK_PATH_DB).c_str(), &dbStat);
        
        if (dbStat.st_size > 0)
        {
            char * readBuffer = new char[dbStat.st_size];
            ifile.read(readBuffer, dbStat.st_size);
            
            char dbVersion = CheckDBVersion(readBuffer);
            if (dbVersion == DB_VERSION)
            {
                ret = LoadDB_Impl(globalHomePath_, readBuffer, dbStat);
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
    SaveDB_Impl(globalHomePath_, fileTree_, threadNum_);
}

char GDBMgr::CheckDBVersion(char * dbBuffer_)
{
    char ret;
    memcpy(&ret, dbBuffer_, 1);
    return ret;
}

GFileTree * GDBMgr::LoadDB_Impl(const char * globalHomePath_, char * dbBuffer_, struct stat & dbStat_)
{
    std::chrono::steady_clock::time_point time_begin = std::chrono::steady_clock::now();
    
    GFile * root = nullptr;
    GFile * parent = nullptr;
    GFile * cur = nullptr;
    long parent_id = -122;
    unsigned char dbSavedSha[SHA1_DIGEST_LENGTH];
    unsigned char dbNowSha[SHA1_DIGEST_LENGTH];
    
    std::map<long, GFile*> gfileMap;
    std::map<long, GFile*>::iterator it;
    
    char * digestBuffer = new char[dbStat_.st_size];
    memset(digestBuffer, 0, dbStat_.st_size);
    size_t offset = 1 + SHA1_DIGEST_LENGTH;
    
    memcpy(dbSavedSha, dbBuffer_ + 1, SHA1_DIGEST_LENGTH);
    
    std::cout << "loading...DB(" CONSOLE_COLOR_FONT_CYAN << GTools::FormatShaToHex(dbSavedSha) << CONSOLE_COLOR_END ")" CONSOLE_COLOR_FONT_YELLOW << GTools::FormatTimestampToYYMMDD_HHMMSS(dbStat_.st_mtimespec.tv_sec) << CONSOLE_COLOR_END << std::endl;
    
    char offsetFormatBuf[FORMAT_FILESIZE_BUFFER_LENGTH];
    char sizeFormatBuf[FORMAT_FILESIZE_BUFFER_LENGTH];
    char timeSpanBuf[30];
    char outputBuf[1024];
    bool outputRunning = true;
    GTools::FormatFileSize(dbStat_.st_size, sizeFormatBuf, CONSOLE_COLOR_FONT_CYAN);
    
    std::thread outputThread(GThreadHelper::PrintLog, outputBuf, &outputRunning);
    
    std::chrono::steady_clock::time_point time_end;
    std::chrono::duration<double> time_span;
    while (offset < dbStat_.st_size)
    {
        time_end = std::chrono::steady_clock::now();
        time_span = std::chrono::duration_cast<std::chrono::duration<double>>(time_end - time_begin);
        
        GTools::FormatFileSize(offset, offsetFormatBuf, CONSOLE_COLOR_FONT_CYAN);
        GTools::FormatTimeduration(time_span.count(), timeSpanBuf);
        sprintf(outputBuf, CONSOLE_CLEAR_LINE "\r(%s/%s)" CONSOLE_COLOR_FONT_YELLOW "%s" CONSOLE_COLOR_END, offsetFormatBuf, sizeFormatBuf, timeSpanBuf);
        
        cur = new GFile();
        offset += cur->FromBin(dbBuffer_ + offset, digestBuffer + offset, &parent_id);
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
    }
    outputRunning = false;
    outputThread.join();
    
    GTools::CalculateSHA1(digestBuffer, dbStat_.st_size, dbNowSha);
    
    time_end = std::chrono::steady_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::duration<double>>(time_end - time_begin);
    
    GTools::FormatTimeduration(time_span.count(), timeSpanBuf);
    
    if (memcmp(dbSavedSha, dbNowSha, SHA1_DIGEST_LENGTH) == 0)
    {
        std::cout << CONSOLE_CLEAR_LINE "\r(" << sizeFormatBuf << ")" CONSOLE_COLOR_FONT_YELLOW << timeSpanBuf << CONSOLE_COLOR_END ".." CONSOLE_COLOR_FONT_GREEN "done" CONSOLE_COLOR_END << std::endl;
    }
    else
    {
        std::cout << CONSOLE_CLEAR_LINE "\r(" << sizeFormatBuf << ")" CONSOLE_COLOR_FONT_YELLOW << timeSpanBuf << CONSOLE_COLOR_END ".." CONSOLE_COLOR_FONT_RED "Incorrect data." CONSOLE_COLOR_END << std::endl;
    }
    
    delete [] digestBuffer;
    
    return new GFileTree(root);
}

void GDBMgr::SaveDB_Impl(const char * globalHomePath_, GFileTree * fileTree_, unsigned threadNum_)
{
    char dbVersion = DB_VERSION;
    std::string homePathStr = globalHomePath_;

    fileTree_->Refresh(true);
    size_t totalLength = fileTree_->CheckBinLength() + SHA1_DIGEST_LENGTH + 1;
    
    char * writeBuffer = new char[totalLength];
    char * digestBuffer = new char[totalLength];
    memset(digestBuffer, 0, totalLength);
    memcpy(writeBuffer, &dbVersion, 1);
    
    fileTree_->ToBin(writeBuffer + 1 + SHA1_DIGEST_LENGTH, digestBuffer + 1 + SHA1_DIGEST_LENGTH, threadNum_);
    
    unsigned char dbSha[SHA1_DIGEST_LENGTH];
    GTools::CalculateSHA1(digestBuffer, totalLength, dbSha);
    
    memcpy(writeBuffer + 1, dbSha, SHA1_DIGEST_LENGTH);
    
    std::ofstream ofile;
    ofile.open((homePathStr + "/" GPARK_PATH_DB).c_str(), std::ios::out | std::ios::binary);
    ofile.write(writeBuffer, totalLength);
    ofile.close();
    
    delete [] writeBuffer;
}

GDBMgr::GDBMgr()
{
    
}

GDBMgr::~GDBMgr()
{
    
}
