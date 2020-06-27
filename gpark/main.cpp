
#include "GPark.h"

int main(int argc, const char * argv[])
{
//    GPark::Instance()->InitDB();
    if (argc == 1)
    {
        GPark::Instance()->Stats();
    }
    else if (argc > 1)
    {
        if (strcmp(argv[1], "init") == 0)
        {
            GPark::Instance()->InitDB();
        }
        else if (strcmp(argv[1], "tree") == 0)
        {
            GPark::Instance()->Tree();
        }
        else
        {
            std::cout << "gpark usage:" << std::endl;
            std::cout << "\tgpark init" << std::endl;
            std::cout << "\tgpark tree" << std::endl;
        }
    }
    
    return 0;
}
