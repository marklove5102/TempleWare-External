#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <vector>
#include "../globals/globals.h"
#include <filesystem>
#include <fstream>
#include <windows.h>
#include <shlobj.h>
#include <json.hpp>

namespace config
{
    class ConfigSystem
    {
    private:
        static std::filesystem::path GetConfigFolder()
        {
            char* userProfile = nullptr;
            size_t len = 0;
            errno_t err = _dupenv_s(&userProfile, &len, "USERPROFILE");

            std::filesystem::path folder;
            if (err != 0 || userProfile == nullptr || len == 0)
            {
                folder = ".templeware";
            }
            else
            {
                folder = userProfile;
                free(userProfile);
                folder /= ".templeware";
            }

            folder /= "external";

            std::error_code ec;
            std::filesystem::create_directories(folder, ec);

            return folder;
        }

        static std::filesystem::path GetConfigPath(const std::string& configName)
        {
            auto folder = GetConfigFolder();
            return folder / (configName + ".json");
        }

    public:

        static std::vector<std::string> ListConfigs()
        {
            std::vector<std::string> list;
            auto folder = GetConfigFolder();

            if (!std::filesystem::exists(folder))
                return list;

            for (auto& entry : std::filesystem::directory_iterator(folder))
            {
                if (entry.is_regular_file())
                {
                    auto path = entry.path();
                    if (path.extension() == ".json")
                    {
                        list.push_back(path.stem().string());
                    }
                }
            }
            return list;
        }

        static void Save(const std::string& configName)
        {
            nlohmann::json j;
            j["TriggerBot"] = globals::TriggerBot;
            j["TriggerBotKey"] = globals::TriggerBotKey;
            j["TriggerBotKeyName"] = std::string(globals::TriggerBotKeyName);
            j["TriggerBotMode"] = globals::TriggerBotMode;
            j["TriggerBotDelay"] = globals::TriggerBotDelay;
            j["TriggerBotToggled"] = globals::TriggerBotToggled;
            j["TriggerBotTeamCheck"] = globals::TriggerBotTeamCheck;
            j["TriggerBotIgnoreFlash"] = globals::TriggerBotIgnoreFlash;

            j["FOV"] = globals::FOV;
            j["Glow"] = globals::Glow;
            j["GlowShowTeam"] = globals::GlowShowTeam;
            j["GlowHealthBased"] = globals::GlowHealthBased;
            j["GlowTeamBased"] = globals::GlowTeamBased;

            j["GlowColor"] = {
                globals::GlowColor.x,
                globals::GlowColor.y,
                globals::GlowColor.z,
                globals::GlowColor.w
            };

            j["GlowTeamColor"] = {
                globals::GlowTeamColor.x,
                globals::GlowTeamColor.y,
                globals::GlowTeamColor.z,
                globals::GlowTeamColor.w
            };

            j["GlowEnemyColor"] = {
                globals::GlowEnemyColor.x,
                globals::GlowEnemyColor.y,
                globals::GlowEnemyColor.z,
                globals::GlowEnemyColor.w
            };

            j["MenuAccentColor"] = {
                globals::MenuAccentColor.x,
                globals::MenuAccentColor.y,
                globals::MenuAccentColor.z,
                globals::MenuAccentColor.w
            };

            j["NoFlashEnabled"] = globals::NoFlashEnabled;
            j["BunnyHopEnabled"] = globals::BunnyHopEnabled;
            j["RCSEnabled"] = globals::RCSEnabled;
            j["RCSStrength"] = globals::RCSStrength;
            j["RCSWhileScoped"] = globals::RCSWhileScoped;
            j["AimbotEnabled"] = globals::AimbotEnabled;
            j["AimbotKey"] = globals::AimbotKey;
            j["AimbotKeyName"] = std::string(globals::AimbotKeyName);
            j["AimbotBone"] = globals::AimbotBone;
            j["AimbotFOV"] = globals::AimbotFOV;
            j["AimbotSmoothness"] = globals::AimbotSmoothness;
            j["AimbotTeamCheck"] = globals::AimbotTeamCheck;

            auto filePath = GetConfigPath(configName);
            std::ofstream ofs(filePath);
            if (ofs.is_open())
            {
                ofs << j.dump(4);
                ofs.close();
            }
        }

        static void Load(const std::string& configName)
        {
            auto filePath = GetConfigPath(configName);
            if (!std::filesystem::exists(filePath))
                return;

            std::ifstream ifs(filePath);
            if (!ifs.is_open())
                return;

            nlohmann::json j;
            ifs >> j;

            globals::TriggerBot = j.value("TriggerBot", false);
            globals::TriggerBotKey = j.value("TriggerBotKey", VK_LSHIFT);
            {
                auto keyNameString = j.value("TriggerBotKeyName", std::string("L-Shift"));
                std::snprintf(globals::TriggerBotKeyName, sizeof(globals::TriggerBotKeyName), "%s", keyNameString.c_str());
            }
            globals::TriggerBotMode = j.value("TriggerBotMode", 0);
            globals::TriggerBotDelay = j.value("TriggerBotDelay", 20);
            globals::TriggerBotToggled = j.value("TriggerBotToggled", false);
            globals::TriggerBotTeamCheck = j.value("TriggerBotTeamCheck", true);
            globals::TriggerBotIgnoreFlash = j.value("TriggerBotIgnoreFlash", false);

            globals::FOV = j.value("FOV", 90);
            globals::Glow = j.value("Glow", false);
            globals::GlowShowTeam = j.value("GlowShowTeam", false);
            globals::GlowHealthBased = j.value("GlowHealthBased", false);
            globals::GlowTeamBased = j.value("GlowTeamBased", false);

            if (j.contains("GlowColor") && j["GlowColor"].is_array())
            {
                auto arr = j["GlowColor"];
                if (arr.size() == 4)
                {
                    globals::GlowColor.x = arr[0].get<float>();
                    globals::GlowColor.y = arr[1].get<float>();
                    globals::GlowColor.z = arr[2].get<float>();
                    globals::GlowColor.w = arr[3].get<float>();
                }
            }

            if (j.contains("GlowTeamColor") && j["GlowTeamColor"].is_array())
            {
                auto arr = j["GlowTeamColor"];
                if (arr.size() == 4)
                {
                    globals::GlowTeamColor.x = arr[0].get<float>();
                    globals::GlowTeamColor.y = arr[1].get<float>();
                    globals::GlowTeamColor.z = arr[2].get<float>();
                    globals::GlowTeamColor.w = arr[3].get<float>();
                }
            }

            if (j.contains("GlowEnemyColor") && j["GlowEnemyColor"].is_array())
            {
                auto arr = j["GlowEnemyColor"];
                if (arr.size() == 4)
                {
                    globals::GlowEnemyColor.x = arr[0].get<float>();
                    globals::GlowEnemyColor.y = arr[1].get<float>();
                    globals::GlowEnemyColor.z = arr[2].get<float>();
                    globals::GlowEnemyColor.w = arr[3].get<float>();
                }
            }

            globals::NoFlashEnabled = j.value("NoFlashEnabled", false);
            globals::BunnyHopEnabled = j.value("BunnyHopEnabled", false);
            globals::RCSEnabled = j.value("RCSEnabled", false);
            globals::RCSStrength = j.value("RCSStrength", 100);
            globals::RCSWhileScoped = j.value("RCSWhileScoped", false);
            globals::AimbotEnabled = j.value("AimbotEnabled", false);
            globals::AimbotKey = j.value("AimbotKey", VK_RBUTTON);
            {
                auto keyNameString = j.value("AimbotKeyName", std::string("R-Button"));
                std::snprintf(globals::AimbotKeyName, sizeof(globals::AimbotKeyName), "%s", keyNameString.c_str());
            }
            globals::AimbotBone = j.value("AimbotBone", 0);
            globals::AimbotFOV = j.value("AimbotFOV", 90);
            globals::AimbotSmoothness = j.value("AimbotSmoothness", 10.0f);
            globals::AimbotTeamCheck = j.value("AimbotTeamCheck", true);

            if (j.contains("MenuAccentColor") && j["MenuAccentColor"].is_array())
            {
                auto arr = j["MenuAccentColor"];
                if (arr.size() == 4)
                {
                    globals::MenuAccentColor.x = arr[0].get<float>();
                    globals::MenuAccentColor.y = arr[1].get<float>();
                    globals::MenuAccentColor.z = arr[2].get<float>();
                    globals::MenuAccentColor.w = arr[3].get<float>();
                }
            }
            ifs.close();
        }

        static void Remove(const std::string& configName)
        {
            auto filePath = GetConfigPath(configName);
            if (std::filesystem::exists(filePath))
            {
                std::error_code ec;
                std::filesystem::remove(filePath, ec);
            }
        }
    };
}
