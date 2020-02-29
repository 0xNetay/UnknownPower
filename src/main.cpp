//
// Created by student on 2/11/20.
//

#include "Launcher/Launcher.hpp"

int main(int argc, char** argv)
{
    Launcher launcher(argc, argv);
    if (!launcher.Init())
    {
        return -1;
    }

    launcher.Run();
    launcher.Close();

    return 0;
}