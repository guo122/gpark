
#ifndef _GTOOLS_H_
#define _GTOOLS_H_

#include "Defines.h"

class GTools
{
public:
    static void FormatFileSize(long long size_, char * str);
    
private:
    GTools();
    ~GTools();
};

#endif /* _GTOOLS_H_ */
