#include "rcs.h"
#include "../memory/memory.h"
#include "../globals/globals.h"
#include "../offsets/offsets.h"
#include <cmath>
#include <thread>
#include <Windows.h>

namespace features
{
    struct Vector3
    {
        float x, y, z;
    };

    static Vector3 oldAimPunch = { 0.0f, 0.0f, 0.0f };

    void RCS::Run(const Memory& memory) noexcept
    {
        if (!globals::RCSEnabled)
        {
            oldAimPunch = { 0.0f, 0.0f, 0.0f };
            return;
        }

        if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000))
        {
            oldAimPunch = { 0.0f, 0.0f, 0.0f };
            return;
        }

        std::uintptr_t localPlayer = memory.Read<std::uintptr_t>(globals::client + offsets::dwLocalPlayerPawn);
        if (!localPlayer)
        {
            oldAimPunch = { 0.0f, 0.0f, 0.0f };
            return;
        }

        int shotsFired = memory.Read<int>(localPlayer + offsets::m_iShotsFired);
        if (shotsFired < 2)
        {
            oldAimPunch = { 0.0f, 0.0f, 0.0f };
            return;
        }

        bool isScoped = memory.Read<bool>(localPlayer + offsets::m_bIsScoped);
        if (isScoped && !globals::RCSWhileScoped)
            return;

        Vector3 currentAimPunch = memory.Read<Vector3>(localPlayer + offsets::m_aimPunchAngle);

        Vector3 punchDelta = {
            currentAimPunch.x - oldAimPunch.x,
            currentAimPunch.y - oldAimPunch.y,
            currentAimPunch.z - oldAimPunch.z
        };

        oldAimPunch = currentAimPunch;

        if (punchDelta.x == 0.0f && punchDelta.y == 0.0f)
            return;

        if (offsets::dwViewAngles == 0)
            return;

        std::uintptr_t viewAnglesAddr = globals::client + offsets::dwViewAngles;
        Vector3 viewAngles = memory.Read<Vector3>(viewAnglesAddr);

        viewAngles.x -= (punchDelta.x * 2.0f) * (globals::RCSStrength / 100.0f);
        viewAngles.y -= (punchDelta.y * 2.0f) * (globals::RCSStrength / 100.0f);

        if (viewAngles.x > 89.0f) viewAngles.x = 89.0f;
        if (viewAngles.x < -89.0f) viewAngles.x = -89.0f;

        while (viewAngles.y > 180.0f) viewAngles.y -= 360.0f;
        while (viewAngles.y < -180.0f) viewAngles.y += 360.0f;

        memory.Write<Vector3>(viewAnglesAddr, viewAngles);
    }
}
