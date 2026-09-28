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
#pragma once

#include "source2toolkit/IToolkitPlugin.h"
#include "source2toolkit/IToolkitApi.h"

// TOOLKIT_SAVEVARS() fills one global per subsystem, so every subsystem header
// has to be in scope for its interface name.
#include "source2toolkit/IToolkitAddresses.h"
#include "source2toolkit/IToolkitCommands.h"
#include "source2toolkit/IToolkitConVars.h"
#include "source2toolkit/IToolkitCustomHud.h"
#include "source2toolkit/IToolkitEntities.h"
#include "source2toolkit/IToolkitEvents.h"
#include "source2toolkit/IToolkitGameConfig.h"
#include "source2toolkit/IToolkitGameSystems.h"
#include "source2toolkit/IToolkitHTTP.h"
#include "source2toolkit/IToolkitJSON.h"
#include "source2toolkit/IToolkitMemory.h"
#include "source2toolkit/IToolkitMenus.h"
#include "source2toolkit/IToolkitHud.h"
#include "source2toolkit/IToolkitModule.h"
#include "source2toolkit/IToolkitMySQL.h"
#include "source2toolkit/IToolkitNetworkMessages.h"
#include "source2toolkit/IToolkitScheduler.h"
#include "source2toolkit/IToolkitSounds.h"
#include "source2toolkit/IToolkitTrace.h"

TOOLKIT_GLOBALVARS();

/**
 * The plugin that serves IToolkitHud: the on-screen texts and elements on
 * the s2t_hud layout, and HudMenu on the s2t_menu layout through the core's
 * menu system. Other plugins fetch the interface with GET_TOOLKIT_IFACE in
 * OnAllToolkitPluginsLoaded().
 */
class HudPlugin final : public IToolkitPlugin,
                        public IToolkitListener
{
public:
    bool Load(PluginId id, IToolkitAPI* api, char* error, size_t maxlen, bool late) override;
    bool Unload(char* error, size_t maxlen) override;

    // IToolkitListener
    void* OnToolkitQuery(const char* iface, int* ret) override;
    void OnGameFrame(bool simulating, bool firstTick, bool lastTick) override;
    void OnLevelShutdown() override;
    void OnClientDisconnect(CPlayerSlot slot, ENetworkDisconnectionReason reason, const char* name, uint64 xuid, const char* networkId) override;

    const char* GetAuthor() override;
    const char* GetName() override;
    const char* GetDescription() override;
    const char* GetVersion() override;
};

extern HudPlugin g_Plugin;
