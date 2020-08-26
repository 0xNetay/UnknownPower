//
// Created by student on 2/11/20.
//

#include "Launcher/Launcher.hpp"

int main(int argc, char** argv)
{
    Launcher launcher(argc, argv);

    if (!launcher.Init())
    {
        printf("Failure during Launcher's Initialization");
        return -1;
    }

    if (!launcher.Run())
    {
        printf("Critical Error during Launcher's Run");
        return -1;
    }

    if (!launcher.Close())
    {
        printf("Failure during Launcher's Close");
        return -1;
    }

    return 0;
}