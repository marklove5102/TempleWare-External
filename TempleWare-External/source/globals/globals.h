#pragma once

#include <cstddef>
#include <locale>
#include <Windows.h>

#include "../../external/imgui/imgui.h"

namespace globals 
{
    
    inline bool TriggerBot = false;       
    inline int TriggerBotKey = VK_LSHIFT;    
    inline char TriggerBotKeyName[64] = "L-Shift"; 
    inline int TriggerBotMode = 0;       
    inline int TriggerBotDelay = 20;           
    inline bool TriggerBotToggled = false;    
    inline bool TriggerBotTeamCheck = true;  
    inline bool TriggerBotIgnoreFlash = false;  

    inline int FOV = 90;
    inline int lastFOV = 90;

    inline bool Glow = false;                     
    inline ImVec4 GlowColor = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
    inline bool GlowShowTeam = false;
    inline bool GlowHealthBased = false;
    inline bool GlowTeamBased = false;
    inline ImVec4 GlowTeamColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    inline ImVec4 GlowEnemyColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    
    inline bool NoFlashEnabled = false;

    inline bool RCSEnabled = false;
    inline int RCSStrength = 100;
    inline bool RCSWhileScoped = false;

    inline bool AimbotEnabled = false;
    inline int AimbotKey = VK_RBUTTON;
    inline char AimbotKeyName[64] = "R-Button";
    inline int AimbotBone = 0;
    inline int AimbotFOV = 90;
    inline float AimbotSmoothness = 10.0f;
    inline bool AimbotTeamCheck = true;          

    inline bool isRunning = true;            
    inline std::uintptr_t client = 0;     

    inline bool BunnyHopEnabled = false;     

    inline bool MenuVisible = true;             
    inline int ShowMenuKey = VK_END; 
    inline ImVec4 MenuAccentColor = ImVec4(0.0f, 0.4f, 1.0f, 1.0f);
}