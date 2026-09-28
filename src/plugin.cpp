/**
 * =============================================================================
 * Source2Toolkit HUD plugin (s2t_hud)
 * Copyright (C) 2025-2026 Michal "Slynx (˙·٠● S l y n x ●٠·˙)" Přikryl
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "plugin.h"

#include "hud.h"
#include "hudmenu.h"

#include <cstring>

HudPlugin g_Plugin;
TOOLKIT_EXPOSE(s2t_hud, g_Plugin);

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

    TOOLKIT_LOG(this, "Serving %s.\n", TOOLKIT_HUD_INTERFACE);
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

    if (!strcmp(iface, TOOLKIT_HUD_INTERFACE))
    {
        if (ret) *ret = TOOLKIT_IFACE_OK;
        return static_cast<IToolkitHud*>(&hud::hudManager);
    }

    // Another revision of the same interface: say so once, so a plugin built
    // against an older or newer SDK finds out why it has no HUD.
    if (!strncmp(iface, "IToolkitHud", 11))
        TOOLKIT_LOG(this, "A plugin asked for %s; this plugin serves %s -- rebuild the plugin or update s2t_hud.\n", iface, TOOLKIT_HUD_INTERFACE);

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
