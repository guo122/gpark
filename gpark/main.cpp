
#include "GPark.h"

int main(int argc, const char * argv[])
{
    if (argc == 1)
    {
        GPark::Instance()->Status(true);
    }
    else if (argc > 1)
    {
        if (strcmp(argv[1], "init") == 0)
        {
            if (argc > 2)
            {
                GPark::Instance()->InitDB(atoi(argv[2]));
            }
            else
            {
                GPark::Instance()->InitDB(0);
            }
        }
        else if (strcmp(argv[1], "status") == 0)
        {
            GPark::Instance()->Status(false);
        }
        else if (strcmp(argv[1], "tree") == 0)
        {
            if (argc > 2)
            {
                GPark::Instance()->Tree(atoi(argv[2]) + 2);
            }
            else
            {
                GPark::Instance()->Tree(-1);
            }
        }
        else if (strcmp(argv[1], "show") == 0)
        {
            if (argc > 2)
            {
                GPark::Instance()->Show(false, atoi(argv[2]) + 2);
            }
            else
            {
                GPark::Instance()->Show(false, -1);
            }
        }
        else if (strcmp(argv[1], "showv") == 0)
        {
            if (argc > 2)
            {
                GPark::Instance()->Show(true, atoi(argv[2]) + 2);
            }
            else
            {
                GPark::Instance()->Show(true, -1);
            }
        }
        else if (strcmp(argv[1], "save") == 0)
        {
            if (argc > 2)
            {
                GPark::Instance()->Save(atoi(argv[2]));
            }
            else
            {
                GPark::Instance()->Save(0);
            }
        }
        else if (argc > 2 && strcmp(argv[1], "diff") == 0)
        {
            GPark::Instance()->Diff(argv[2]);
        }
        else
        {
            // todo(gzy): opt arguments.
            std::cout << "gpark " MAIN_VERSION " usage:" << std::endl;
            std::cout << "\tgpark init [threads]" << std::endl;
            std::cout << "\tgpark status" << std::endl;
            std::cout << "\tgpark tree [depth]" << std::endl;
            std::cout << "\tgpark show [depth]" << std::endl;
            std::cout << "\tgpark showv [depth]" << std::endl;
            std::cout << "\tgpark save [threads]" << std::endl;
            std::cout << "\tgpark diff <other repos>" << std::endl;
        }
    }

    GPark::Instance()->Destory();

    return 0;
}
