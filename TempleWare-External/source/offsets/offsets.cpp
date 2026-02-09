#include "offsets.h"
#include <string>
#include <Windows.h>
#include <array>
#include <regex>
#include <iostream>

bool Get(std::string url, std::string& response)
{
    std::string cmd = "curl -s -X GET " + url;

    std::array<char, 128> buffer;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);

    if (!pipe)
        return false;

    response.clear();
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr)
        response += buffer.data();

    if (response.empty())
        return false;

    std::regex pattern("\\d{3}:");
    if (std::regex_search(response, pattern))
        return false;

    return true;
}

bool offsets::UpdateOffset()
{
    const std::string offsetsUrl    = "https://raw.githubusercontent.com/a2x/cs2-dumper/main/output/offsets.json";
    const std::string buttonsUrl    = "https://raw.githubusercontent.com/a2x/cs2-dumper/main/output/buttons.json";
    const std::string client_dllUrl = "https://raw.githubusercontent.com/a2x/cs2-dumper/main/output/client_dll.json";
    std::string offsetsData, buttonsData, client_dllData;

    if (!Get(offsetsUrl, offsetsData))
    {
        std::cerr << "[ERROR] Failed to fetch offsets.json" << std::endl;
        return false;
    }
    if (!Get(buttonsUrl, buttonsData))
    {
        std::cerr << "[ERROR] Failed to fetch buttons.json" << std::endl;
        return false;
    }
    if (!Get(client_dllUrl, client_dllData))
    {
        std::cerr << "[ERROR] Failed to fetch client_dll.json" << std::endl;
        return false;
    }

    json offsetsJson, client_dllJson, buttonsJson;
    try {
        offsetsJson = json::parse(offsetsData);
        json client_dllFull = json::parse(client_dllData);
        if (!client_dllFull.contains("client.dll") || !client_dllFull["client.dll"].contains("classes"))
        {
            std::cerr << "[ERROR] Invalid client_dll.json structure" << std::endl;
            return false;
        }
        client_dllJson = client_dllFull["client.dll"]["classes"];
        buttonsJson = json::parse(buttonsData);
    }
    catch (const json::exception& e)
    {
        std::cerr << "[ERROR] JSON parsing failed: " << e.what() << std::endl;
        return false;
    }

    auto GetOffset = [](const json& j, const std::string& path) -> std::ptrdiff_t {
        try {
            if (j.contains(path) && j[path].is_number())
                return j[path].get<std::ptrdiff_t>();
        }
        catch (...) {}
        return 0;
    };

    auto GetFieldOffset = [](const json& classes, const std::string& className, const std::string& fieldName) -> std::ptrdiff_t {
        try {
            if (classes.contains(className) && 
                classes[className].contains("fields") && 
                classes[className]["fields"].contains(fieldName))
            {
                return classes[className]["fields"][fieldName].get<std::ptrdiff_t>();
            }
        }
        catch (...) {}
        return 0;
    };

    dwLocalPlayerPawn = GetOffset(offsetsJson["client.dll"], "dwLocalPlayerPawn");
    dwLocalPlayerController = GetOffset(offsetsJson["client.dll"], "dwLocalPlayerController");
    dwEntityList = GetOffset(offsetsJson["client.dll"], "dwEntityList");
    dwViewMatrix = GetOffset(offsetsJson["client.dll"], "dwViewMatrix");
    dwViewAngles = GetOffset(offsetsJson["client.dll"], "dwViewAngles");
    dwCSGOInput = GetOffset(offsetsJson["client.dll"], "dwCSGOInput");
    
    m_ViewAngles = GetFieldOffset(client_dllJson, "C_CSGOInput", "m_ViewAngles");

    m_pCameraServices = GetFieldOffset(client_dllJson, "C_BasePlayerPawn", "m_pCameraServices");
    m_glowColorOverride = GetFieldOffset(client_dllJson, "CGlowProperty", "m_glowColorOverride");
    m_iFOV = GetFieldOffset(client_dllJson, "CCSPlayerBase_CameraServices", "m_iFOV");
    m_bGlowing = GetFieldOffset(client_dllJson, "CGlowProperty", "m_bGlowing");
    m_bIsScoped = GetFieldOffset(client_dllJson, "C_CSPlayerPawn", "m_bIsScoped");
    attack = GetOffset(buttonsJson["client.dll"], "attack");
    m_iIDEntIndex = GetFieldOffset(client_dllJson, "C_CSPlayerPawn", "m_iIDEntIndex");
    flFlashDuration = GetFieldOffset(client_dllJson, "C_CSPlayerPawnBase", "m_flFlashDuration");
    m_iShotsFired = GetFieldOffset(client_dllJson, "C_CSPlayerPawn", "m_iShotsFired");
    m_aimPunchAngle = GetFieldOffset(client_dllJson, "C_CSPlayerPawn", "m_aimPunchAngle");

    m_hPawn = GetFieldOffset(client_dllJson, "CBasePlayerController", "m_hPawn");
    m_hPlayerPawn = GetFieldOffset(client_dllJson, "CCSPlayerController", "m_hPlayerPawn");
    m_Glow = GetFieldOffset(client_dllJson, "C_BaseModelEntity", "m_Glow");
    m_iHealth = GetFieldOffset(client_dllJson, "C_BaseEntity", "m_iHealth");
    m_iTeamNum = GetFieldOffset(client_dllJson, "C_BaseEntity", "m_iTeamNum");
    m_vOldOrigin = GetFieldOffset(client_dllJson, "C_BasePlayerPawn", "m_vOldOrigin");
    m_entitySpottedState = GetFieldOffset(client_dllJson, "C_CSPlayerPawn", "m_entitySpottedState");
    m_vecViewOffset = GetFieldOffset(client_dllJson, "C_BaseModelEntity", "m_vecViewOffset");
    m_fFlags = GetFieldOffset(client_dllJson, "C_BaseEntity", "m_fFlags");
    
    m_bIsLocalPlayerController = GetFieldOffset(client_dllJson, "CBasePlayerController", "m_bIsLocalPlayerController");

    if (dwLocalPlayerPawn == 0 || dwLocalPlayerController == 0 || dwEntityList == 0 || 
        dwViewAngles == 0 || m_iHealth == 0 || m_iTeamNum == 0)
    {
        std::cerr << "[ERROR] Critical offsets are zero or invalid" << std::endl;
        std::cerr << "  dwLocalPlayerPawn: " << dwLocalPlayerPawn << std::endl;
        std::cerr << "  dwLocalPlayerController: " << dwLocalPlayerController << std::endl;
        std::cerr << "  dwEntityList: " << dwEntityList << std::endl;
        std::cerr << "  dwViewAngles: " << dwViewAngles << std::endl;
        return false;
    }

    std::cout << "[SUCCESS] Offsets loaded successfully" << std::endl;
    return true;
}