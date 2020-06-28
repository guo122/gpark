
#include "GPark.h"

int main(int argc, const char * argv[])
{
//    GPark::Instance()->Save();
    if (argc == 1)
    {
        GPark::Instance()->Status();
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
        else if (strcmp(argv[1], "show") == 0)
        {
            if (argc > 2 && strcmp(argv[2], "-v") == 0)
            {
                GPark::Instance()->Show(true);
            }
            else
            {
                GPark::Instance()->Show(false);
            }
        }
        else if (strcmp(argv[1], "save") == 0)
        {
            GPark::Instance()->Save();
        }
        else if (argc > 2 && strcmp(argv[1], "diff") == 0)
        {
            GPark::Instance()->Diff(argv[2]);
        }
        else
        {
            std::cout << "gpark usage:" << std::endl;
            std::cout << "\tgpark init" << std::endl;
            std::cout << "\tgpark tree" << std::endl;
            std::cout << "\tgpark show [-v]" << std::endl;
            std::cout << "\tgpark save" << std::endl;
            std::cout << "\tgpark diff <other repos>" << std::endl;
        }
    }
    
    std::cout << std::endl;
    
    GPark::Instance()->Destory();
    
    return 0;
}
