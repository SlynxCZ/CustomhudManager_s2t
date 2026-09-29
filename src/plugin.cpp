/**
 * vim: set ts=4 sw=4 tw=99 noet:
 * =============================================================================
 * CustomhudManager_s2t
 * Copyright (C) 2026 Michal "Slynx (˙·٠● S l y n x ●٠·˙)" Přikryl.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, Michal "Slynx (˙·٠● S l y n x ●٠·˙)" Přikryl gives
 * you permission to link the code of this program (as well as its derivative
 * works) to "Counter-Strike 2," "Source 2," "Steam," and any Game MODs or
 * server software running on software by Valve Corporation. You must obey the
 * GNU General Public License in all respects for all other code used.
 *
 * Additionally, this exception applies to all derivative works unless
 * otherwise stated in LICENSE.txt.
 *
 * Authors:
 *   - Michal "Slynx (˙·٠● S l y n x ●٠·˙)" Přikryl
 *
 * Project: CustomhudManager_s2t
 */
#include "plugin.h"

#include "hud.h"
#include "hudmenu.h"

#include <cstring>

#define VERSION_STRING SEMVER " @ " GITHUB_SHA

HudPlugin g_Plugin;
TOOLKIT_EXPOSE(CustomhudManager, g_Plugin);

bool HudPlugin::Load(PluginId id, IToolkitAPI* api, char* error, size_t maxlen, bool late)
{
    TOOLKIT_SAVEVARS();

    if (!g_pToolkitCustomHud || !g_pToolkitMenus || !g_pToolkitEntities)
    {
        if (error && maxlen)
            g_ToolkitAPI->Format(error, maxlen, "the core is missing an interface this plugin needs (custom HUD, menus, entities): update Source2Toolkit");
        return false;
    }

    // OnGameFrame, OnLevelShutdown, OnClientDisconnect, OnToolkitQuery.
    api->AddListener(this, this);

    hud::hudManager.Init();

    return true;
}

bool HudPlugin::Unload(char* error, size_t maxlen)
{
    // The instances' code is in this library: nothing of them may stay open.
    hudmenu::CloseAll();
    hud::hudManager.Clear();
    return true;
}

void* HudPlugin::OnToolkitQuery(const char* iface, int* ret)
{
    if (!iface)
        return nullptr;

    if (!strcmp(iface, CUSTOMHUD_MANAGER_INTERFACE_VERSION))
    {
        if (ret) *ret = TOOLKIT_IFACE_OK;
        return static_cast<ICustomhudManager*>(&hud::hudManager);
    }

    if (ret) *ret = TOOLKIT_IFACE_FAILED;

    return nullptr;
}

void HudPlugin::OnGameFrame(bool simulating, bool firstTick, bool lastTick)
{
    hud::hudManager.Tick();
}

void HudPlugin::OnLevelShutdown()
{
    // The layouts do not survive the level change.
    hud::hudManager.Clear();
}

void HudPlugin::OnClientDisconnect(CPlayerSlot slot, ENetworkDisconnectionReason reason, const char* name, uint64 xuid, const char* networkId)
{
    hud::hudManager.OnClientDisconnect(slot);
}

const char* HudPlugin::GetAuthor()
{
    return "Slynx (˙·٠● S l y n x ●٠·˙)";
}

const char* HudPlugin::GetName()
{
    return "CustomHUD Manager";
}

const char* HudPlugin::GetDescription()
{
    return "On-screen texts, HUD elements and menus on the Panorama HUD (ICustomhudManager)";
}

const char* HudPlugin::GetVersion()
{
    return VERSION_STRING;
}
