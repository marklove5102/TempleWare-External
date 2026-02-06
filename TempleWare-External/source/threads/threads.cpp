#include "threads.h"
#include "../features/fov.h"
#include "../features/noflash.h"
#include "../features/triggerbot.h"
#include "../menu/menu.h"
#include "../features/bhop.h"
#include "../features/glow.h"
#include "../features/rcs.h"
#include "../features/aimbot.h"

#include "../offsets/offsets.h"
#include "../globals/globals.h"

#include <thread>

namespace threads {
    void RunMiscThread(const Memory& memory) noexcept 
    {
        while (gui::isRunning) 
        {
            features::FOVManager::AdjustFOV(memory);
            features::Bhop::Run(memory);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void RunVisualThread(const Memory& memory) noexcept 
    {
        while (gui::isRunning) 
        {
            features::NoFlash::Run(memory);
            features::Glow::Run(memory);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void RunAimThread(const Memory& memory) noexcept
    {
        while (gui::isRunning)
        {
            if (globals::TriggerBot)
                features::TriggerBot::Run(memory);
            if (globals::RCSEnabled)
                features::RCS::Run(memory);
            if (globals::AimbotEnabled)
                features::Aimbot::Run(memory);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}
