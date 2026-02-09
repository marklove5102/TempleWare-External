#include "menu/menu.h"
#include "memory/memory.h"
#include "globals/globals.h"
#include "threads/threads.h"
#include "offsets/offsets.h"

#include <thread>

int __stdcall wWinMain(HINSTANCE instance, HINSTANCE previousInstance, PWSTR arguments, int commandShow)
{
    if (!offsets::UpdateOffset())
    {
        MessageBoxA(nullptr, "Failed to load offsets. Please check your internet connection and try again.", "TempleWare External - Error", MB_OK | MB_ICONERROR);
        return EXIT_FAILURE;
    }

    static const auto memory = Memory("cs2.exe");

    if (!memory.IsValid())
    {
        DWORD error = GetLastError();
        char errorMsg[256];
        snprintf(errorMsg, sizeof(errorMsg), 
            "Failed to attach to cs2.exe.\n\n"
            "Please ensure:\n"
            "1. Counter-Strike 2 is running\n"
            "2. You have sufficient permissions (try running as Administrator)\n"
            "3. No antivirus is blocking the application\n\n"
            "Error code: %lu", error);
        MessageBoxA(nullptr, errorMsg, "TempleWare External - Error", MB_OK | MB_ICONERROR);
        return EXIT_FAILURE;
    }

    globals::client = memory.GetModuleAddress("client.dll");
    if (globals::client == 0)
    {
        MessageBoxA(nullptr, "Failed to find client.dll module. Please make sure Counter-Strike 2 is running.", "TempleWare External - Error", MB_OK | MB_ICONERROR);
        return EXIT_FAILURE;
    }

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