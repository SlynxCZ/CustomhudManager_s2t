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
#pragma once
#include "source2toolkit/IToolkitHud.h"

#include "ehandle.h"
#include "playerslot.h"

#include <functional>
#include <string>

class CCSCustomHudLayout;
class CCSPlayerController;

namespace hud
{
    /// One custom_hud_layout the plugin owns: created on first use once a map
    /// runs, gone with the map, never recreated after a failure until the
    /// next map (the failure is logged once).
    struct OwnedLayout
    {
        std::string name;
        const char* targetName = nullptr;
        CHandle<CCSCustomHudLayout> handle;
        bool failed = false;
        /// Runs right after a fresh entity spawned.
        std::function<void(CCSCustomHudLayout*)> onCreated;
    };

    class HudManager final : public IToolkitHud
    {
    public:
        static constexpr int kMaxPlayers = 64;
        static constexpr int kBarSteps = 20;
        static constexpr int kToasts = 4;
        static constexpr int kChips = 4;
        static constexpr int kFeedRows = 5;
        /// How long the hit flash and its number stay.
        static constexpr float kHitSeconds = 0.9f;

        // IToolkitHud
        bool IsAvailable() override;
        void ShowText(CCSPlayerController* player, HudSlot slot, const char* text, float seconds, HudTextStyle style) override;
        void HideText(CCSPlayerController* player, HudSlot slot) override;
        void ShowPrompt(CCSPlayerController* player, const char* key, const char* text, float progress) override;
        void HidePrompt(CCSPlayerController* player) override;
        void HideAll(CCSPlayerController* player) override;
        const char* LayoutName() override;

        // IToolkitHud002
        void ShowToast(CCSPlayerController* player, HudToastStyle style, const char* title, const char* text, float seconds) override;
        void ClearToasts(CCSPlayerController* player) override;
        void ShowAnnounce(CCSPlayerController* player, const char* title, const char* subtitle, float seconds, HudColor color) override;
        void HideAnnounce(CCSPlayerController* player) override;
        void ShowCountdown(CCSPlayerController* player, const char* text, float seconds, HudColor color) override;
        void HideCountdown(CCSPlayerController* player) override;
        void ShowStatus(CCSPlayerController* player, int chip, const char* label, const char* value, HudColor color) override;
        void HideStatus(CCSPlayerController* player, int chip) override;
        void ShowProgress(CCSPlayerController* player, const char* label, const char* value, float progress, HudColor color) override;
        void HideProgress(CCSPlayerController* player) override;
        void ShowHit(CCSPlayerController* player, int damage, bool headshot, bool kill) override;
        void AddFeed(CCSPlayerController* player, HudToastStyle style, const char* time, const char* text, float seconds) override;
        void ClearFeed(CCSPlayerController* player) override;
        void ShowOverlay(CCSPlayerController* player, HudOverlay overlay, const char* text, float seconds) override;
        void HideOverlay(CCSPlayerController* player) override;

        // IToolkitHud004
        void OpenMenu(PluginId owner, CCSPlayerController* player, HudMenu* menu) override;
        void ShowTimer(CCSPlayerController* player, const char* tag, const char* time, const char* sub, HudColor color) override;
        void HideTimer(CCSPlayerController* player) override;
        void ShowCard(CCSPlayerController* player, const char* tag, const char* title, const char* sub, HudColor color) override;
        void HideCard(CCSPlayerController* player) override;

        // plugin
        void Init();

        /// The text layout, spawned if it can be; null without a map or after
        /// a failed spawn.
        CCSCustomHudLayout* TextLayout();

        /// The menu layout ("s2t_menu"), same rules. Clicks on it go to
        /// hudmenu::OnLayoutClick.
        CCSCustomHudLayout* MenuLayout();

        /// The menu layout only if it already exists -- for closing.
        CCSCustomHudLayout* MenuLayoutIfAny();

        /// Once a frame: whatever has a time is hidden when it is up.
        void Tick();

        /// The slot's state is reset so the next occupant does not inherit it.
        void OnClientDisconnect(CPlayerSlot slot);

        /// Level shutdown: the entities are gone, forget them and every state.
        void Clear();

    private:
        /// Something shown for a while: the text of a slot, a toast, the
        /// announcement, ... `variant` is the index of the class currently on
        /// the panel out of a family (colour, style), -1 for none.
        struct Timed
        {
            bool shown = false;
            float expire = -1.0f;
            int variant = -1;
        };

        struct SlotState : Timed
        {
            int size = -1;
            /// Index into the offset class tables (0 = -10 steps), -1 unknown.
            int offX = -1;
            int offY = -1;
        };

        /// A toast or a feed row keeps its texts, so the stack can shift
        /// when a new one comes in.
        struct Card : Timed
        {
            std::string title;
            std::string text;
        };

        struct PlayerState
        {
            SlotState slots[static_cast<int>(HudSlot::Count)];
            bool prompt = false;
            bool bar = false;
            int barStep = -1;

            Card toasts[kToasts];
            bool toastAnim = false;
            Timed announce;
            bool announceAnim = false;
            Timed countdown;
            bool countdownAnim = false;
            Timed chips[kChips];
            bool statusShown = false;
            Timed progress;
            int progressStep = -1;
            Timed hit;
            bool hitAnim = false;
            bool hitHeadshot = false;
            bool hitKill = false;
            bool damageShown = false;
            Card feed[kFeedRows];
            bool feedShown = false;
            Timed overlay;
            Timed timer;
            bool timerAnim = false;
            Timed card;
        };

        CCSCustomHudLayout* Ensure(OwnedLayout& layout);
        PlayerState* StateOf(CCSPlayerController* player);
        void ResetState(PlayerState& state);

        /// Swaps one class of a family (c-*, s-*, t-*, o-*, w*) on a panel:
        /// the old one off, the new one on. `current` remembers which is on.
        void SetVariant(CCSCustomHudLayout* layout, CCSPlayerController* player, const char* panel,
                        const char* const* classes, int count, int& current, int wanted);

        /// Flips the a/b pair that restarts an entrance animation.
        void Restart(CCSCustomHudLayout* layout, CCSPlayerController* player, const char* panel,
                     const char* classA, const char* classB, bool& flag);

        /// Draws a toast or feed card into its panel from what the state holds.
        void DrawCard(CCSCustomHudLayout* layout, CCSPlayerController* player, const char* panel,
                      const char* titleLabel, const char* textLabel, Card& card, bool show);

        OwnedLayout m_text;
        OwnedLayout m_menu;
        PlayerState m_players[kMaxPlayers];
    };

    extern HudManager hudManager;
}
