
#include "GPark.h"
#include "GTools.h"

int main(int argc, const char * argv[])
{
    if (argc == 1)
    {
        GPark::Instance()->Status(true, 0);
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
            GPark::Instance()->Status(false, 0);
        }
        else if (strcmp(argv[1], "tree") == 0)
        {
            if (argc > 3)
            {
                GPark::Instance()->Tree(atoi(argv[2]) + 2, atoi(argv[3]));
            }
            else if (argc > 2)
            {
                GPark::Instance()->Tree(atoi(argv[2]) + 2, 0);
            }
            else
            {
                GPark::Instance()->Tree(-1, 0);
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
        else if (strcmp(argv[1], "info") == 0)
        {
            std::cout << "hardware thread num: " CONSOLE_COLOR_FONT_YELLOW << GTools::HardwareThreadNum() << CONSOLE_COLOR_END << std::endl;
        }
        else
        {
            // todo(gzy): opt arguments.
            std::cout << "gpark " MAIN_VERSION " usage:" << std::endl;
            std::cout << "\tgpark init [threads]" << std::endl;
            std::cout << "\tgpark status" << std::endl;
            std::cout << "\tgpark tree [depth] [threads]" << std::endl;
            std::cout << "\tgpark show [depth] [threads]" << std::endl;
            std::cout << "\tgpark showv [depth] [threads]" << std::endl;
            std::cout << "\tgpark save [threads]" << std::endl;
            std::cout << "\tgpark diff <other repos>" << std::endl;
            std::cout << "\tgpark info" << std::endl;
        }
    }

    GPark::Instance()->Destory();

    return 0;
}
