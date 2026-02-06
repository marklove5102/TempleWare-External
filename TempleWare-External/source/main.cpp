/************************************************************************************************
 *                                          Temple Cheat                                        *
 *                     (c) 2023-2024 Temple Client Cheats Team. All rights reserved.            *
 ************************************************************************************************/

#include "menu/menu.h"
#include "memory/memory.h"
#include "globals/globals.h"
#include "threads/threads.h"
#include "offsets/offsets.h"

#include <thread>

int __stdcall wWinMain(HINSTANCE instance, HINSTANCE previousInstance, PWSTR arguments, int commandShow)
{
    if (!offsets::UpdateOffset())
        return EXIT_FAILURE;

    static const auto memory = Memory("cs2.exe");

    globals::client = memory.GetModuleAddress("client.dll");

    std::thread(threads::RunMiscThread, std::ref(memory)).detach();
    std::thread(threads::RunVisualThread, std::ref(memory)).detach();
    std::thread(threads::RunAimThread, std::ref(memory)).detach();

    gui::CreateHWindow("templecheats.xyz");
    gui::CreateDevice();
    gui::CreateImGui();

    bool windowVisible = true;
    static bool endKeyPressed = false;

    while (globals::isRunning && gui::isRunning)
    {
        bool endKeyDown = (GetAsyncKeyState(VK_END) & 0x8000) != 0;
        bool insertKeyDown = (GetAsyncKeyState(VK_INSERT) & 0x8000) != 0;
        bool homeKeyDown = (GetAsyncKeyState(VK_HOME) & 0x8000) != 0;

        if (endKeyDown && !endKeyPressed)
        {
            endKeyPressed = true;
            globals::isRunning = false;
            gui::isRunning = false;
            break;
        }
        else if (!endKeyDown)
        {
            endKeyPressed = false;
        }

        if (insertKeyDown || homeKeyDown)
        {
            windowVisible = !windowVisible;
            ShowWindow(gui::window, windowVisible ? SW_SHOW : SW_HIDE);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        if (!globals::isRunning || !gui::isRunning)
            break;

        if (windowVisible) 
        {
            gui::BeginRender();
            gui::Render();
            gui::EndRender();
        }
        else
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    globals::isRunning = false;
    gui::isRunning = false;

    gui::DestroyImGui();
    gui::DestroyDevice();
    gui::DestroyHWindow();

    return EXIT_SUCCESS;
}