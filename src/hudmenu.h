/**
 * =============================================================================
 * CustomhudManager_s2t -- the Source2Toolkit Panorama HUD plugin
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
 *
 * As a special exception, the author gives you permission to link the code of
 * this program (as well as its derivative works) to "Counter-Strike 2,"
 * "Source 2," "Steam," and any Game MODs or server software running on
 * software by Valve Corporation. You must obey the GNU General Public License
 * in all respects for all other code used.
 */
#pragma once
#include "source2toolkit/IToolkitHud.h"

#include <cstdint>

class CCSCustomHudLayout;
class CCSPlayerController;

namespace hudmenu
{
    /// A HudMenu on the plugin's menu layout: six rows, the navigation
    /// buttons, per-player state on one entity. Built by HudManager::OpenMenu
    /// and handed to the core (IToolkitMenus::OpenMenu), which draws it, feeds
    /// it the keys and destroys it -- through Destroy(), so here.
    class HudMenuInstance final : public IMenuInstance
    {
    public:
        static constexpr int kRows = 6;

        HudMenuInstance(CCSPlayerController* player, HudMenu* menu);
        ~HudMenuInstance() override;

        void Display() override;
        void OnKeyPress(CCSPlayerController* player, int key) override;
        void Close() override;

        /// Once a frame. A HUD menu does not fade, so it is not redrawn every
        /// frame like the center HTML one; this runs the DisabledEvaluators
        /// and redraws only when what would be drawn changed -- which also
        /// covers a menu rebuilt in place from one of its handlers, and the
        /// player dying (CaptureWhenDead).
        void OnFrame() override;

        /// The keys are dropped only in the mode the player chose the mouse
        /// for. A dead player's temporary cursor keeps them.
        bool AcceptsKeys() const override { return !hudMenu_->CaptureInput; }

        /// A click on the menu layout for this player: the button id becomes
        /// the key the row or navigation button stands for.
        void OnClick(const char* buttonId);

    protected:
        int NumPerPage() const override { return kRows; }

    private:
        /// Whether the layout takes the mouse right now: the menu's own
        /// setting, or the player being dead with CaptureWhenDead.
        bool EffectiveCapture() const;

        /// Hides the layout for the player; the destructor's job, so a
        /// closed, replaced or unloaded menu never stays on screen.
        void Hide();

        /// What Display() would draw, hashed: title, page, rows, states.
        size_t Signature() const;

        int Position() const { return static_cast<int>(hudMenu_->Position); }

        HudMenu* hudMenu_;
        uint64_t serial_;
        int slot_;
        size_t drawn_ = 0;
        /// The pos-* class last put on menu_root for this player; -1 for none yet.
        int drawnPos_ = -1;
    };

    /// Opens `menu` for the player through the core's menu system.
    void Open(PluginId owner, CCSPlayerController* player, HudMenu* menu);

    /// The click callback of the menu layout: routes to the player's instance.
    void OnLayoutClick(CCSPlayerController* player, const char* buttonId);

    /// Whether the menu opened under this serial is still the one the player
    /// has open -- after an option handler ran, which may have closed it.
    bool IsOpen(int slot, uint64_t serial);

    /// Closes every HUD menu that is open: the plugin is unloading and the
    /// instances' code goes with it.
    void CloseAll();
}
