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
 */
#include "hud.h"

#include "hudmenu.h"
#include "plugin.h"

#include "source2toolkit/schema/entity/classes/CCSCustomHudLayout.h"
#include "source2toolkit/schema/entity/classes/CCSPlayerController.h"

#include <algorithm>
#include <chrono>
#include <cmath>

namespace hud
{
    HudManager hudManager;

    namespace
    {
        // The id contract of IToolkitHud.h, in HudSlot order.
        constexpr const char* kSlotPanel[] = {
            "hud_top", "hud_topleft", "hud_topright", "hud_left", "hud_right", "hud_center", "hud_bottom", "hud_panel",
        };
        static_assert(sizeof(kSlotPanel) / sizeof(kSlotPanel[0]) == static_cast<size_t>(HudSlot::Count));

        constexpr const char* kSlotLabel[] = {
            "hud_top_text", "hud_topleft_text", "hud_topright_text", "hud_left_text", "hud_right_text", "hud_center_text",
            "hud_bottom_text", "hud_panel_text",
        };

        constexpr const char* kColorClass[] = {
            "c-white", "c-red", "c-green", "c-blue", "c-yellow", "c-orange", "c-purple", "c-cyan", "c-grey",
        };
        static_assert(sizeof(kColorClass) / sizeof(kColorClass[0]) == static_cast<size_t>(HudColor::Count));

        constexpr const char* kSizeClass[] = { "s-small", "s-normal", "s-large", "s-huge" };
        static_assert(sizeof(kSizeClass) / sizeof(kSizeClass[0]) == static_cast<size_t>(HudSize::Count));

        // HudTextStyle::offsetX/offsetY, -10..10, as classes on the slot panel.
        constexpr int kOffsetRange = 10;
        constexpr int kOffsetClasses = 2 * kOffsetRange + 1;
        constexpr const char* kOffXClass[kOffsetClasses] = {
            "xm10", "xm9", "xm8", "xm7", "xm6", "xm5", "xm4", "xm3", "xm2", "xm1", "x0",
            "xp1", "xp2", "xp3", "xp4", "xp5", "xp6", "xp7", "xp8", "xp9", "xp10",
        };
        constexpr const char* kOffYClass[kOffsetClasses] = {
            "ym10", "ym9", "ym8", "ym7", "ym6", "ym5", "ym4", "ym3", "ym2", "ym1", "y0",
            "yp1", "yp2", "yp3", "yp4", "yp5", "yp6", "yp7", "yp8", "yp9", "yp10",
        };

        int OffsetIndex(int offset)
        {
            return (std::max)(-kOffsetRange, (std::min)(kOffsetRange, offset)) + kOffsetRange;
        }

        constexpr const char* kToastClass[] = { "t-info", "t-success", "t-warning", "t-danger", "t-neutral" };
        static_assert(sizeof(kToastClass) / sizeof(kToastClass[0]) == static_cast<size_t>(HudToastStyle::Count));

        constexpr const char* kOverlayClass[] = { "o-poison", "o-burn", "o-freeze", "o-heal", "o-blind", "o-black" };
        static_assert(sizeof(kOverlayClass) / sizeof(kOverlayClass[0]) == static_cast<size_t>(HudOverlay::Count));

        // w0 .. w20: a bar's fill in 5 % steps, one class each (clip in the
        // stylesheet, which does not restart like a width transition would).
        constexpr const char* kBarClass[] = {
            "w0", "w1", "w2", "w3", "w4", "w5", "w6", "w7", "w8", "w9", "w10",
            "w11", "w12", "w13", "w14", "w15", "w16", "w17", "w18", "w19", "w20",
        };
        static_assert(sizeof(kBarClass) / sizeof(kBarClass[0]) == HudManager::kBarSteps + 1);

        constexpr const char* kToastPanel[] = { "hud_toast_0", "hud_toast_1", "hud_toast_2", "hud_toast_3" };
        constexpr const char* kToastTitle[] = { "hud_toast_0_title", "hud_toast_1_title", "hud_toast_2_title", "hud_toast_3_title" };
        constexpr const char* kToastText[] = { "hud_toast_0_text", "hud_toast_1_text", "hud_toast_2_text", "hud_toast_3_text" };
        static_assert(sizeof(kToastPanel) / sizeof(kToastPanel[0]) == HudManager::kToasts);

        constexpr const char* kChipPanel[] = { "hud_status_0", "hud_status_1", "hud_status_2", "hud_status_3" };
        constexpr const char* kChipLabel[] = { "hud_status_0_label", "hud_status_1_label", "hud_status_2_label", "hud_status_3_label" };
        constexpr const char* kChipValue[] = { "hud_status_0_value", "hud_status_1_value", "hud_status_2_value", "hud_status_3_value" };
        static_assert(sizeof(kChipPanel) / sizeof(kChipPanel[0]) == HudManager::kChips);

        constexpr const char* kFeedPanel[] = { "hud_feed_0", "hud_feed_1", "hud_feed_2", "hud_feed_3", "hud_feed_4" };
        constexpr const char* kFeedTime[] = { "hud_feed_0_time", "hud_feed_1_time", "hud_feed_2_time", "hud_feed_3_time", "hud_feed_4_time" };
        constexpr const char* kFeedText[] = { "hud_feed_0_text", "hud_feed_1_text", "hud_feed_2_text", "hud_feed_3_text", "hud_feed_4_text" };
        static_assert(sizeof(kFeedPanel) / sizeof(kFeedPanel[0]) == HudManager::kFeedRows);

        constexpr const char* kShow = "show";
        constexpr const char* kText = "text";

        // A steady clock, as the toolkit's scheduler uses: the engine's
        // curtime through the SDK's CGlobalVars did not advance on a live
        // server and nothing timed ever went away.
        float Now()
        {
            using namespace std::chrono;
            static const steady_clock::time_point start = steady_clock::now();
            return duration<float>(steady_clock::now() - start).count();
        }

        float ExpireAt(float seconds)
        {
            return seconds > 0.0f ? Now() + seconds : -1.0f;
        }

        int SlotOf(CCSPlayerController* player)
        {
            if (!player)
                return -1;

            const int slot = player->GetPlayerSlot().Get();
            return slot >= 0 && slot < HudManager::kMaxPlayers ? slot : -1;
        }

        int BarStep(float progress)
        {
            return std::clamp(static_cast<int>(std::lround((std::min)(progress, 1.0f) * HudManager::kBarSteps)), 0, HudManager::kBarSteps);
        }

        const char* Safe(const char* text)
        {
            return text ? text : "";
        }
    }

    // ---- setup ---------------------------------------------------------------

    void HudManager::Init()
    {
        m_text.name = "s2t_hud";
        m_text.targetName = "s2t_hud_text";

        m_menu.name = "s2t_menu";
        m_menu.targetName = "s2t_hud_menu";
        m_menu.onCreated = [](CCSCustomHudLayout* layout)
        {
            // Owned by this plugin: the core drops the callback when the
            // plugin unloads, and it goes with the entity otherwise.
            g_pToolkitCustomHud->HookCustomHudClick(g_PluginID, layout,
                [](CCSPlayerController* player, CCSCustomHudLayout*, const char* buttonId)
                {
                    hudmenu::OnLayoutClick(player, buttonId);
                });
        };
    }

    CCSCustomHudLayout* HudManager::Ensure(OwnedLayout& layout)
    {
        if (CCSCustomHudLayout* existing = layout.handle.Get())
            return existing;

        if (layout.failed || layout.name.empty())
            return nullptr;

        // No entities before the map's game rules exist.
        if (!g_pToolkitEntities->GetGameRules())
            return nullptr;

        CCSCustomHudLayout* created = CCSCustomHudLayout::Create(layout.name.c_str(), layout.targetName);
        if (!created)
        {
            layout.failed = true;
            TOOLKIT_LOG(&g_Plugin, "custom_hud_layout '%s' could not be spawned; HUD texts and menus are off until the next map\n", layout.name.c_str());
            return nullptr;
        }

        layout.handle = created->GetHandle();
        TOOLKIT_LOG(&g_Plugin, "custom_hud_layout '%s' spawned (%s)\n", layout.name.c_str(), layout.targetName ? layout.targetName : "");

        if (layout.onCreated)
            layout.onCreated(created);

        return created;
    }

    CCSCustomHudLayout* HudManager::TextLayout()
    {
        return Ensure(m_text);
    }

    CCSCustomHudLayout* HudManager::MenuLayout()
    {
        return Ensure(m_menu);
    }

    CCSCustomHudLayout* HudManager::MenuLayoutIfAny()
    {
        return m_menu.handle.Get();
    }

    HudManager::PlayerState* HudManager::StateOf(CCSPlayerController* player)
    {
        const int slot = SlotOf(player);
        return slot < 0 ? nullptr : &m_players[slot];
    }

    void HudManager::ResetState(PlayerState& state)
    {
        state = PlayerState{};
    }

    void HudManager::SetVariant(CCSCustomHudLayout* layout, CCSPlayerController* player, const char* panel,
                                const char* const* classes, int count, int& current, int wanted)
    {
        if (wanted < 0 || wanted >= count)
            wanted = 0;

        if (current == wanted)
            return;

        if (current >= 0 && current < count)
            layout->SetHasClass(panel, classes[current], false, player);
        else
        {
            // Nothing is known about this panel for this player -- the slot
            // was reused or the state reset -- so whatever class is still on
            // it from before goes, or two of them would fight in the stylesheet.
            for (int i = 0; i < count; ++i)
                if (i != wanted)
                    layout->SetHasClass(panel, classes[i], false, player);
        }

        layout->SetHasClass(panel, classes[wanted], true, player);
        current = wanted;
    }

    void HudManager::Restart(CCSCustomHudLayout* layout, CCSPlayerController* player, const char* panel,
                             const char* classA, const char* classB, bool& flag)
    {
        // Turning one class off and on in the same tick sends nothing; two
        // names used alternately do.
        flag = !flag;
        layout->SetHasClass(panel, classA, flag, player);
        layout->SetHasClass(panel, classB, !flag, player);
    }

    void HudManager::DrawCard(CCSCustomHudLayout* layout, CCSPlayerController* player, const char* panel,
                              const char* titleLabel, const char* textLabel, Card& card, bool show)
    {
        if (show)
        {
            layout->SetDialogVariableString(titleLabel, kText, card.title.c_str(), player);
            layout->SetDialogVariableString(textLabel, kText, card.text.c_str(), player);
        }

        // Always sent, not only on a change: a client still loading when the
        // class was first set never saw it.
        layout->SetHasClass(panel, kShow, show, player);
        card.shown = show;
    }

    // ---- IToolkitHud ---------------------------------------------------------

    bool HudManager::IsAvailable()
    {
        return TextLayout() != nullptr;
    }

    const char* HudManager::LayoutName()
    {
        return m_text.name.c_str();
    }

    void HudManager::ShowText(CCSPlayerController* player, HudSlot slot, const char* text, float seconds, HudTextStyle style)
    {
        const int index = static_cast<int>(slot);
        if (index < 0 || index >= static_cast<int>(HudSlot::Count) || !text)
            return;

        PlayerState* state = StateOf(player);
        CCSCustomHudLayout* layout = TextLayout();
        if (!state || !layout)
            return;

        SlotState& s = state->slots[index];

        SetVariant(layout, player, kSlotPanel[index], kColorClass, static_cast<int>(HudColor::Count), s.variant, static_cast<int>(style.color));
        SetVariant(layout, player, kSlotPanel[index], kSizeClass, static_cast<int>(HudSize::Count), s.size, static_cast<int>(style.size));
        SetVariant(layout, player, kSlotPanel[index], kOffXClass, kOffsetClasses, s.offX, OffsetIndex(style.offsetX));
        SetVariant(layout, player, kSlotPanel[index], kOffYClass, kOffsetClasses, s.offY, OffsetIndex(style.offsetY));

        layout->SetDialogVariableString(kSlotLabel[index], kText, text, player);
        layout->SetHasClass(kSlotPanel[index], kShow, true, player);
        s.shown = true;
        s.expire = ExpireAt(seconds);
    }

    void HudManager::HideText(CCSPlayerController* player, HudSlot slot)
    {
        const int index = static_cast<int>(slot);
        if (index < 0 || index >= static_cast<int>(HudSlot::Count))
            return;

        PlayerState* state = StateOf(player);
        if (!state)
            return;

        SlotState& s = state->slots[index];
        if (!s.shown)
            return;

        s.shown = false;
        s.expire = -1.0f;

        if (CCSCustomHudLayout* layout = m_text.handle.Get())
            layout->SetHasClass(kSlotPanel[index], kShow, false, player);
    }

    void HudManager::ShowPrompt(CCSPlayerController* player, const char* key, const char* text, float progress)
    {
        PlayerState* state = StateOf(player);
        CCSCustomHudLayout* layout = TextLayout();
        if (!state || !layout)
            return;

        layout->SetDialogVariableString("hud_prompt_key", kText, Safe(key), player);
        layout->SetDialogVariableString("hud_prompt_text", kText, Safe(text), player);

        if (progress >= 0.0f)
        {
            SetVariant(layout, player, "hud_prompt_bar", kBarClass, kBarSteps + 1, state->barStep, BarStep(progress));

            if (!state->bar)
            {
                layout->SetHasClass("hud_prompt_barwrap", kShow, true, player);
                state->bar = true;
            }
        }
        else if (state->bar)
        {
            layout->SetHasClass("hud_prompt_barwrap", kShow, false, player);
            state->bar = false;
        }

        layout->SetHasClass("hud_prompt", kShow, true, player);
        state->prompt = true;
    }

    void HudManager::HidePrompt(CCSPlayerController* player)
    {
        PlayerState* state = StateOf(player);
        if (!state || !state->prompt)
            return;

        state->prompt = false;

        if (CCSCustomHudLayout* layout = m_text.handle.Get())
        {
            layout->SetHasClass("hud_prompt", kShow, false, player);

            if (state->bar)
                layout->SetHasClass("hud_prompt_barwrap", kShow, false, player);
        }

        state->bar = false;
    }

    void HudManager::HideAll(CCSPlayerController* player)
    {
        for (int i = 0; i < static_cast<int>(HudSlot::Count); ++i)
            HideText(player, static_cast<HudSlot>(i));

        HidePrompt(player);
        ClearToasts(player);
        HideAnnounce(player);
        HideCountdown(player);
        HideStatus(player, -1);
        HideProgress(player);
        ClearFeed(player);
        HideOverlay(player);
        HideTimer(player);
        HideCard(player);
        HideTimer(player);
        HideCard(player);

        if (PlayerState* state = StateOf(player))
        {
            if (state->hit.shown)
            {
                if (CCSCustomHudLayout* layout = m_text.handle.Get())
                {
                    layout->SetHasClass("hud_hitmarker", kShow, false, player);
                    layout->SetHasClass("hud_damage", kShow, false, player);
                }
                state->hit.shown = false;
                state->damageShown = false;
            }
        }
    }

    // ---- IToolkitHud002 ------------------------------------------------------

    void HudManager::ShowToast(CCSPlayerController* player, HudToastStyle style, const char* title, const char* text, float seconds)
    {
        PlayerState* state = StateOf(player);
        CCSCustomHudLayout* layout = TextLayout();
        if (!state || !layout)
            return;

        // The stack shifts down: every card takes the one above it, and the
        // newest goes to the top. Texts and styles are redrawn for the cards
        // that moved.
        for (int i = kToasts - 1; i > 0; --i)
        {
            Card& below = state->toasts[i];
            Card& above = state->toasts[i - 1];

            const int wasVariant = below.variant;
            below.title = above.title;
            below.text = above.text;
            below.expire = above.expire;
            below.variant = wasVariant;

            if (above.shown)
                SetVariant(layout, player, kToastPanel[i], kToastClass, static_cast<int>(HudToastStyle::Count), below.variant, above.variant);

            DrawCard(layout, player, kToastPanel[i], kToastTitle[i], kToastText[i], below, above.shown);
        }

        Card& top = state->toasts[0];
        top.title = Safe(title);
        top.text = Safe(text);
        top.expire = ExpireAt(seconds);
        SetVariant(layout, player, kToastPanel[0], kToastClass, static_cast<int>(HudToastStyle::Count), top.variant, static_cast<int>(style));
        DrawCard(layout, player, kToastPanel[0], kToastTitle[0], kToastText[0], top, true);
        Restart(layout, player, kToastPanel[0], "in-a", "in-b", state->toastAnim);
    }

    void HudManager::ClearToasts(CCSPlayerController* player)
    {
        PlayerState* state = StateOf(player);
        if (!state)
            return;

        CCSCustomHudLayout* layout = m_text.handle.Get();

        for (int i = 0; i < kToasts; ++i)
        {
            Card& card = state->toasts[i];
            if (card.shown && layout)
                layout->SetHasClass(kToastPanel[i], kShow, false, player);

            card.shown = false;
            card.expire = -1.0f;
        }
    }

    void HudManager::ShowAnnounce(CCSPlayerController* player, const char* title, const char* subtitle, float seconds, HudColor color)
    {
        PlayerState* state = StateOf(player);
        CCSCustomHudLayout* layout = TextLayout();
        if (!state || !layout)
            return;

        layout->SetDialogVariableString("hud_announce_title", kText, Safe(title), player);
        layout->SetDialogVariableString("hud_announce_sub", kText, Safe(subtitle), player);
        SetVariant(layout, player, "hud_announce", kColorClass, static_cast<int>(HudColor::Count), state->announce.variant, static_cast<int>(color));
        layout->SetHasClass("hud_announce", kShow, true, player);
        Restart(layout, player, "hud_announce", "in-a", "in-b", state->announceAnim);

        state->announce.shown = true;
        state->announce.expire = ExpireAt(seconds);
    }

    void HudManager::HideAnnounce(CCSPlayerController* player)
    {
        PlayerState* state = StateOf(player);
        if (!state || !state->announce.shown)
            return;

        state->announce.shown = false;
        state->announce.expire = -1.0f;

        if (CCSCustomHudLayout* layout = m_text.handle.Get())
            layout->SetHasClass("hud_announce", kShow, false, player);
    }

    void HudManager::ShowCountdown(CCSPlayerController* player, const char* text, float seconds, HudColor color)
    {
        PlayerState* state = StateOf(player);
        CCSCustomHudLayout* layout = TextLayout();
        if (!state || !layout)
            return;

        layout->SetDialogVariableString("hud_countdown_text", kText, Safe(text), player);
        SetVariant(layout, player, "hud_countdown", kColorClass, static_cast<int>(HudColor::Count), state->countdown.variant, static_cast<int>(color));
        layout->SetHasClass("hud_countdown", kShow, true, player);
        Restart(layout, player, "hud_countdown", "pop-a", "pop-b", state->countdownAnim);

        state->countdown.shown = true;
        state->countdown.expire = ExpireAt(seconds);
    }

    void HudManager::HideCountdown(CCSPlayerController* player)
    {
        PlayerState* state = StateOf(player);
        if (!state || !state->countdown.shown)
            return;

        state->countdown.shown = false;
        state->countdown.expire = -1.0f;

        if (CCSCustomHudLayout* layout = m_text.handle.Get())
            layout->SetHasClass("hud_countdown", kShow, false, player);
    }

    void HudManager::ShowStatus(CCSPlayerController* player, int chip, const char* label, const char* value, HudColor color)
    {
        if (chip < 0 || chip >= kChips)
            return;

        PlayerState* state = StateOf(player);
        CCSCustomHudLayout* layout = TextLayout();
        if (!state || !layout)
            return;

        Timed& c = state->chips[chip];
        layout->SetDialogVariableString(kChipLabel[chip], kText, Safe(label), player);
        layout->SetDialogVariableString(kChipValue[chip], kText, Safe(value), player);
        SetVariant(layout, player, kChipPanel[chip], kColorClass, static_cast<int>(HudColor::Count), c.variant, static_cast<int>(color));
        layout->SetHasClass(kChipPanel[chip], kShow, true, player);
        c.shown = true;

        if (!state->statusShown)
        {
            layout->SetHasClass("hud_status", kShow, true, player);
            state->statusShown = true;
        }
    }

    void HudManager::HideStatus(CCSPlayerController* player, int chip)
    {
        PlayerState* state = StateOf(player);
        if (!state)
            return;

        CCSCustomHudLayout* layout = m_text.handle.Get();
        bool any = false;

        for (int i = 0; i < kChips; ++i)
        {
            Timed& c = state->chips[i];
            if (chip >= 0 && i != chip)
            {
                any = any || c.shown;
                continue;
            }

            if (c.shown && layout)
                layout->SetHasClass(kChipPanel[i], kShow, false, player);
            c.shown = false;
        }

        if (!any && state->statusShown)
        {
            if (layout)
                layout->SetHasClass("hud_status", kShow, false, player);
            state->statusShown = false;
        }
    }

    void HudManager::ShowProgress(CCSPlayerController* player, const char* label, const char* value, float progress, HudColor color)
    {
        PlayerState* state = StateOf(player);
        CCSCustomHudLayout* layout = TextLayout();
        if (!state || !layout)
            return;

        layout->SetDialogVariableString("hud_progress_label", kText, Safe(label), player);
        layout->SetDialogVariableString("hud_progress_value", kText, Safe(value), player);
        SetVariant(layout, player, "hud_progress", kColorClass, static_cast<int>(HudColor::Count), state->progress.variant, static_cast<int>(color));
        SetVariant(layout, player, "hud_progress_bar", kBarClass, kBarSteps + 1, state->progressStep, BarStep((std::max)(progress, 0.0f)));
        layout->SetHasClass("hud_progress", kShow, true, player);
        state->progress.shown = true;
    }

    void HudManager::HideProgress(CCSPlayerController* player)
    {
        PlayerState* state = StateOf(player);
        if (!state || !state->progress.shown)
            return;

        state->progress.shown = false;

        if (CCSCustomHudLayout* layout = m_text.handle.Get())
            layout->SetHasClass("hud_progress", kShow, false, player);
    }

    void HudManager::ShowHit(CCSPlayerController* player, int damage, bool headshot, bool kill)
    {
        PlayerState* state = StateOf(player);
        CCSCustomHudLayout* layout = TextLayout();
        if (!state || !layout)
            return;

        if (state->hitHeadshot != headshot)
        {
            layout->SetHasClass("hud_hitmarker", "headshot", headshot, player);
            layout->SetHasClass("hud_damage", "headshot", headshot, player);
            state->hitHeadshot = headshot;
        }
        if (state->hitKill != kill)
        {
            layout->SetHasClass("hud_hitmarker", "kill", kill, player);
            layout->SetHasClass("hud_damage", "kill", kill, player);
            state->hitKill = kill;
        }

        layout->SetHasClass("hud_hitmarker", kShow, true, player);
        Restart(layout, player, "hud_hitmarker", "hit-a", "hit-b", state->hitAnim);

        const bool number = damage > 0;
        if (number)
        {
            layout->SetDialogVariableString("hud_damage_text", kText, std::to_string(damage).c_str(), player);
            layout->SetHasClass("hud_damage", kShow, true, player);
            // The same flag: the number and the flash restart together.
            layout->SetHasClass("hud_damage", "dmg-a", state->hitAnim, player);
            layout->SetHasClass("hud_damage", "dmg-b", !state->hitAnim, player);
        }
        else if (state->damageShown)
        {
            layout->SetHasClass("hud_damage", kShow, false, player);
        }

        state->damageShown = number;
        state->hit.shown = true;
        state->hit.expire = Now() + kHitSeconds;
    }

    void HudManager::AddFeed(CCSPlayerController* player, HudToastStyle style, const char* time, const char* text, float seconds)
    {
        PlayerState* state = StateOf(player);
        CCSCustomHudLayout* layout = TextLayout();
        if (!state || !layout)
            return;

        for (int i = kFeedRows - 1; i > 0; --i)
        {
            Card& below = state->feed[i];
            Card& above = state->feed[i - 1];

            const int wasVariant = below.variant;
            below.title = above.title;
            below.text = above.text;
            below.expire = above.expire;
            below.variant = wasVariant;

            if (above.shown)
                SetVariant(layout, player, kFeedPanel[i], kToastClass, static_cast<int>(HudToastStyle::Count), below.variant, above.variant);

            DrawCard(layout, player, kFeedPanel[i], kFeedTime[i], kFeedText[i], below, above.shown);
        }

        Card& top = state->feed[0];
        top.title = Safe(time);
        top.text = Safe(text);
        top.expire = ExpireAt(seconds);
        SetVariant(layout, player, kFeedPanel[0], kToastClass, static_cast<int>(HudToastStyle::Count), top.variant, static_cast<int>(style));
        DrawCard(layout, player, kFeedPanel[0], kFeedTime[0], kFeedText[0], top, true);

        if (!state->feedShown)
        {
            layout->SetHasClass("hud_feed", kShow, true, player);
            state->feedShown = true;
        }
    }

    void HudManager::ClearFeed(CCSPlayerController* player)
    {
        PlayerState* state = StateOf(player);
        if (!state)
            return;

        CCSCustomHudLayout* layout = m_text.handle.Get();

        for (int i = 0; i < kFeedRows; ++i)
        {
            Card& row = state->feed[i];
            if (row.shown && layout)
                layout->SetHasClass(kFeedPanel[i], kShow, false, player);

            row.shown = false;
            row.expire = -1.0f;
        }

        if (state->feedShown)
        {
            if (layout)
                layout->SetHasClass("hud_feed", kShow, false, player);
            state->feedShown = false;
        }
    }

    void HudManager::ShowOverlay(CCSPlayerController* player, HudOverlay overlay, const char* text, float seconds)
    {
        PlayerState* state = StateOf(player);
        CCSCustomHudLayout* layout = TextLayout();
        if (!state || !layout)
            return;

        layout->SetDialogVariableString("hud_overlay_text", kText, Safe(text), player);
        SetVariant(layout, player, "hud_overlay", kOverlayClass, static_cast<int>(HudOverlay::Count), state->overlay.variant, static_cast<int>(overlay));
        layout->SetHasClass("hud_overlay", kShow, true, player);

        state->overlay.shown = true;
        state->overlay.expire = ExpireAt(seconds);
    }

    void HudManager::HideOverlay(CCSPlayerController* player)
    {
        PlayerState* state = StateOf(player);
        if (!state || !state->overlay.shown)
            return;

        state->overlay.shown = false;
        state->overlay.expire = -1.0f;

        if (CCSCustomHudLayout* layout = m_text.handle.Get())
            layout->SetHasClass("hud_overlay", kShow, false, player);
    }

    // ---- IToolkitHud004 ------------------------------------------------------

    void HudManager::OpenMenu(PluginId owner, CCSPlayerController* player, HudMenu* menu)
    {
        hudmenu::Open(owner, player, menu);
    }

    void HudManager::ShowTimer(CCSPlayerController* player, const char* tag, const char* time, const char* sub, HudColor color)
    {
        PlayerState* state = StateOf(player);
        CCSCustomHudLayout* layout = TextLayout();
        if (!state || !layout)
            return;

        layout->SetDialogVariableString("hud_timer_tag", kText, Safe(tag), player);
        layout->SetDialogVariableString("hud_timer_num", kText, Safe(time), player);
        layout->SetDialogVariableString("hud_timer_sub", kText, Safe(sub), player);
        layout->SetHasClass("hud_timer", "has-tag", tag && *tag, player);
        layout->SetHasClass("hud_timer", "has-sub", sub && *sub, player);
        SetVariant(layout, player, "hud_timer", kColorClass, static_cast<int>(HudColor::Count), state->timer.variant, static_cast<int>(color));
        layout->SetHasClass("hud_timer", kShow, true, player);
        Restart(layout, player, "hud_timer", "tick-a", "tick-b", state->timerAnim);

        state->timer.shown = true;
        state->timer.expire = -1.0f;
    }

    void HudManager::HideTimer(CCSPlayerController* player)
    {
        PlayerState* state = StateOf(player);
        if (!state || !state->timer.shown)
            return;

        state->timer.shown = false;
        state->timer.expire = -1.0f;

        if (CCSCustomHudLayout* layout = m_text.handle.Get())
            layout->SetHasClass("hud_timer", kShow, false, player);
    }

    void HudManager::ShowCard(CCSPlayerController* player, const char* tag, const char* title, const char* sub, HudColor color)
    {
        PlayerState* state = StateOf(player);
        CCSCustomHudLayout* layout = TextLayout();
        if (!state || !layout)
            return;

        layout->SetDialogVariableString("hud_card_tag", kText, Safe(tag), player);
        layout->SetDialogVariableString("hud_card_title", kText, Safe(title), player);
        layout->SetDialogVariableString("hud_card_sub", kText, Safe(sub), player);
        layout->SetHasClass("hud_card", "has-tag", tag && *tag, player);
        layout->SetHasClass("hud_card", "has-sub", sub && *sub, player);
        SetVariant(layout, player, "hud_card", kColorClass, static_cast<int>(HudColor::Count), state->card.variant, static_cast<int>(color));
        layout->SetHasClass("hud_card", kShow, true, player);

        state->card.shown = true;
        state->card.expire = -1.0f;
    }

    void HudManager::HideCard(CCSPlayerController* player)
    {
        PlayerState* state = StateOf(player);
        if (!state || !state->card.shown)
            return;

        state->card.shown = false;
        state->card.expire = -1.0f;

        if (CCSCustomHudLayout* layout = m_text.handle.Get())
            layout->SetHasClass("hud_card", kShow, false, player);
    }

    // ---- plugin ----------------------------------------------------------------

    void HudManager::Tick()
    {
        if (!m_text.handle.Get())
            return;

        const float now = Now();

        for (int slot = 0; slot < kMaxPlayers; ++slot)
        {
            PlayerState& state = m_players[slot];
            CCSPlayerController* player = nullptr;

            const auto due = [now](const Timed& t) { return t.shown && t.expire >= 0.0f && now >= t.expire; };
            const auto controller = [&]() -> CCSPlayerController*
            {
                if (!player)
                    player = CCSPlayerController::FromSlot(slot);
                return player;
            };

            for (int i = 0; i < static_cast<int>(HudSlot::Count); ++i)
            {
                if (!due(state.slots[i]))
                    continue;

                if (controller())
                    HideText(player, static_cast<HudSlot>(i));
                else
                    state.slots[i] = SlotState{};
            }

            for (int i = 0; i < kToasts; ++i)
            {
                Card& card = state.toasts[i];
                if (!due(card))
                    continue;

                if (CCSCustomHudLayout* layout = controller() ? m_text.handle.Get() : nullptr)
                    layout->SetHasClass(kToastPanel[i], kShow, false, player);
                card.shown = false;
                card.expire = -1.0f;
            }

            for (int i = 0; i < kFeedRows; ++i)
            {
                Card& row = state.feed[i];
                if (!due(row))
                    continue;

                if (CCSCustomHudLayout* layout = controller() ? m_text.handle.Get() : nullptr)
                    layout->SetHasClass(kFeedPanel[i], kShow, false, player);
                row.shown = false;
                row.expire = -1.0f;
            }

            if (due(state.announce))
                controller() ? HideAnnounce(player) : (void)(state.announce = Timed{});
            if (due(state.countdown))
                controller() ? HideCountdown(player) : (void)(state.countdown = Timed{});
            if (due(state.overlay))
                controller() ? HideOverlay(player) : (void)(state.overlay = Timed{});

            if (due(state.hit))
            {
                if (CCSCustomHudLayout* layout = controller() ? m_text.handle.Get() : nullptr)
                {
                    layout->SetHasClass("hud_hitmarker", kShow, false, player);
                    if (state.damageShown)
                        layout->SetHasClass("hud_damage", kShow, false, player);
                }
                state.hit.shown = false;
                state.hit.expire = -1.0f;
                state.damageShown = false;
            }
        }
    }

    void HudManager::OnClientDisconnect(CPlayerSlot slot)
    {
        const int index = slot.Get();
        if (index < 0 || index >= kMaxPlayers)
            return;

        // The controller is still there on the disconnect callback, so the
        // entity's per-slot state can be cleared for the next occupant.
        if (CCSPlayerController* player = CCSPlayerController::FromSlot(index))
            HideAll(player);

        ResetState(m_players[index]);
    }

    void HudManager::Clear()
    {
        m_text.handle = nullptr;
        m_text.failed = false;
        m_menu.handle = nullptr;
        m_menu.failed = false;

        for (PlayerState& state : m_players)
            ResetState(state);
    }
}
