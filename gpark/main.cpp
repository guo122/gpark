
#include "GPark.h"

int main(int argc, const char * argv[])
{
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
        else if (strcmp(argv[1], "show") == 0)
        {
            GPark::Instance()->Show();
        }
        else if (strcmp(argv[1], "save") == 0)
        {
            GPark::Instance()->Save();
        }
        else
        {
            std::cout << "gpark usage:" << std::endl;
            std::cout << "\tgpark init" << std::endl;
            std::cout << "\tgpark tree" << std::endl;
            std::cout << "\tgpark show" << std::endl;
            std::cout << "\tgpark save" << std::endl;
        }
    }
    
    GPark::Instance()->Destory();
    
    return 0;
}
