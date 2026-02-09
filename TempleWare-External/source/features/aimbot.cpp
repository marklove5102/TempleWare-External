#include "aimbot.h"
#include "../memory/memory.h"
#include "../globals/globals.h"
#include "../offsets/offsets.h"
#include <cmath>
#include <Windows.h>
#include <algorithm>

namespace features
{
    struct Vector3
    {
        float x, y, z;
    };

    struct Vector2
    {
        float x, y;
    };

    static float GetDistance3D(const Vector3& a, const Vector3& b)
    {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        float dz = a.z - b.z;
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }

    static Vector3 CalculateAngle(const Vector3& from, const Vector3& to)
    {
        Vector3 delta = { to.x - from.x, to.y - from.y, to.z - from.z };
        float distance = sqrtf(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);

        Vector3 angle;
        angle.x = -asinf(delta.z / distance) * (180.0f / 3.14159265f);
        angle.y = atan2f(delta.y, delta.x) * (180.0f / 3.14159265f);
        angle.z = 0.0f;

        return angle;
    }

    static float GetFOV(const Vector3& viewAngles, const Vector3& targetAngles)
    {
        Vector3 delta = {
            targetAngles.x - viewAngles.x,
            targetAngles.y - viewAngles.y,
            0.0f
        };

        while (delta.y > 180.0f) delta.y -= 360.0f;
        while (delta.y < -180.0f) delta.y += 360.0f;

        return sqrtf(delta.x * delta.x + delta.y * delta.y);
    }

    void Aimbot::Run(const Memory& memory) noexcept
    {
        if (!globals::AimbotEnabled)
            return;

        bool keyDown = (GetAsyncKeyState(globals::AimbotKey) & 0x8000) != 0;
        if (!keyDown)
            return;

        if (globals::client == 0)
            return;

        std::uintptr_t localPlayer = memory.Read<std::uintptr_t>(globals::client + offsets::dwLocalPlayerPawn);
        if (!localPlayer)
            return;

        std::uintptr_t localPlayerController = memory.Read<std::uintptr_t>(globals::client + offsets::dwLocalPlayerController);
        if (!localPlayerController)
            return;

        int localTeam = memory.Read<int>(localPlayerController + offsets::m_iTeamNum);
        Vector3 localViewOffset = memory.Read<Vector3>(localPlayer + offsets::m_vecViewOffset);
        Vector3 localOrigin = memory.Read<Vector3>(localPlayer + offsets::m_vOldOrigin);
        Vector3 localEyePos = { localOrigin.x + localViewOffset.x, localOrigin.y + localViewOffset.y, localOrigin.z + localViewOffset.z };

        if (offsets::dwCSGOInput == 0)
            return;

        std::uintptr_t csgoInput = memory.Read<std::uintptr_t>(globals::client + offsets::dwCSGOInput);
        if (!csgoInput)
            return;

        std::ptrdiff_t viewAnglesOffset = offsets::dwViewAngles - offsets::dwCSGOInput;
        std::uintptr_t viewAnglesAddr = csgoInput + viewAnglesOffset;
        
        Vector3 viewAngles = memory.Read<Vector3>(viewAnglesAddr);

        std::uintptr_t entityList = memory.Read<std::uintptr_t>(globals::client + offsets::dwEntityList);
        if (!entityList)
            return;

        float bestFOV = static_cast<float>(globals::AimbotFOV);
        Vector3 bestAngle = { 0.0f, 0.0f, 0.0f };
        bool foundTarget = false;

        for (int i = 1; i < 64; ++i)
        {
            std::uintptr_t listEntry = memory.Read<std::uintptr_t>(entityList + (8 * (i >> 9)) + 16);
            if (!listEntry)
                continue;

            std::uintptr_t controller = memory.Read<std::uintptr_t>(listEntry + 112 * (i & 0x1FF));
            if (!controller)
                continue;

            bool isLocalPlayer = memory.Read<bool>(controller + offsets::m_bIsLocalPlayerController);
            if (isLocalPlayer)
                continue;

            int team = memory.Read<int>(controller + offsets::m_iTeamNum);
            if (globals::AimbotTeamCheck && team == localTeam)
                continue;

            std::uint32_t playerPawn = memory.Read<std::uint32_t>(controller + offsets::m_hPlayerPawn);
            if (!playerPawn)
                continue;

            std::uintptr_t listEntry2 = memory.Read<std::uintptr_t>(entityList + 8 * ((playerPawn & 0x7FFF) >> 9) + 16);
            if (!listEntry2)
                continue;

            std::uintptr_t playerPawnEntity = memory.Read<std::uintptr_t>(listEntry2 + 112 * (playerPawn & 0x1FF));
            if (!playerPawnEntity)
                continue;

            int health = memory.Read<int>(playerPawnEntity + offsets::m_iHealth);
            if (health < 1)
                continue;

            Vector3 targetOrigin = memory.Read<Vector3>(playerPawnEntity + offsets::m_vOldOrigin);
            Vector3 targetViewOffset = memory.Read<Vector3>(playerPawnEntity + offsets::m_vecViewOffset);

            Vector3 targetBonePos;
            if (globals::AimbotBone == 0)
            {
                targetBonePos = { targetOrigin.x + targetViewOffset.x, targetOrigin.y + targetViewOffset.y, targetOrigin.z + targetViewOffset.z };
            }
            else if (globals::AimbotBone == 1)
            {
                targetBonePos = { targetOrigin.x + targetViewOffset.x, targetOrigin.y + targetViewOffset.y, targetOrigin.z + targetViewOffset.z - 5.0f };
            }
            else
            {
                targetBonePos = { targetOrigin.x, targetOrigin.y, targetOrigin.z + targetViewOffset.z - 10.0f };
            }

            Vector3 targetAngle = CalculateAngle(localEyePos, targetBonePos);
            float fov = GetFOV(viewAngles, targetAngle);

            if (fov < bestFOV)
            {
                bestFOV = fov;
                bestAngle = targetAngle;
                foundTarget = true;
            }
        }

        if (foundTarget)
        {
            Vector3 delta = {
                bestAngle.x - viewAngles.x,
                bestAngle.y - viewAngles.y,
                0.0f
            };

            float smoothness = globals::AimbotSmoothness;
            if (smoothness < 1.0f) smoothness = 1.0f;

            viewAngles.x += delta.x / smoothness;
            viewAngles.y += delta.y / smoothness;

            if (viewAngles.x > 89.0f) viewAngles.x = 89.0f;
            if (viewAngles.x < -89.0f) viewAngles.x = -89.0f;

            while (viewAngles.y > 180.0f) viewAngles.y -= 360.0f;
            while (viewAngles.y < -180.0f) viewAngles.y += 360.0f;

            memory.Write<Vector3>(viewAnglesAddr, viewAngles);
        }
    }
}
