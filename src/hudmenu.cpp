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
#include "hudmenu.h"

#include "hud.h"
#include "plugin.h"

#include "source2toolkit/schema/entity/classes/CCSCustomHudLayout.h"
#include "source2toolkit/schema/entity/classes/CCSPlayerController.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <string>
#include <unordered_map>

namespace hudmenu
{
    namespace
    {
        constexpr const char* kRowPanel[HudMenuInstance::kRows] = {
            "menu_row_0", "menu_row_1", "menu_row_2", "menu_row_3", "menu_row_4", "menu_row_5",
        };
        constexpr const char* kRowKey[HudMenuInstance::kRows] = {
            "menu_row_0_key", "menu_row_1_key", "menu_row_2_key", "menu_row_3_key", "menu_row_4_key", "menu_row_5_key",
        };
        constexpr const char* kRowText[HudMenuInstance::kRows] = {
            "menu_row_0_text", "menu_row_1_text", "menu_row_2_text", "menu_row_3_text", "menu_row_4_text", "menu_row_5_text",
        };
        constexpr const char* kShow = "show";
        constexpr const char* kDisabled = "disabled";

        /// The instance each slot has open, and its serial. The core owns the
        /// instances; this is only how a click and a handler's aftermath find
        /// theirs. An instance registers in its constructor and leaves in its
        /// destructor -- only if it is still the one registered, since the
        /// core builds the replacement before it destroys the old one.
        std::unordered_map<int, HudMenuInstance*> s_bySlot;
        std::unordered_map<int, uint64_t> s_serialBySlot;
        uint64_t s_nextSerial = 0;

        void HashIn(size_t& seed, size_t value)
        {
            seed ^= value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
        }
    }

    bool IsOpen(int slot, uint64_t serial)
    {
        auto it = s_serialBySlot.find(slot);
        return it != s_serialBySlot.end() && it->second == serial;
    }

    HudMenuInstance::HudMenuInstance(CCSPlayerController* player, HudMenu* menu)
        : IMenuInstance(player, menu), hudMenu_(menu), serial_(++s_nextSerial), slot_(player ? player->GetSlot() : -1)
    {
        if (slot_ >= 0)
        {
            s_bySlot[slot_] = this;
            s_serialBySlot[slot_] = serial_;
        }
    }

    HudMenuInstance::~HudMenuInstance()
    {
        Hide();

        if (slot_ >= 0)
        {
            auto it = s_bySlot.find(slot_);
            if (it != s_bySlot.end() && it->second == this)
            {
                s_bySlot.erase(it);
                s_serialBySlot.erase(slot_);
            }
        }
    }

    bool HudMenuInstance::EffectiveCapture() const
    {
        if (hudMenu_->CaptureInput)
            return true;

        if (!hudMenu_->CaptureWhenDead)
            return false;

        CCSPlayerController* player = slot_ >= 0 ? CCSPlayerController::FromSlot(slot_) : nullptr;
        return player && !player->m_bPawnIsAlive();
    }

    void HudMenuInstance::Hide()
    {
        CCSCustomHudLayout* layout = hud::hudManager.MenuLayoutIfAny();
        CCSPlayerController* player = slot_ >= 0 ? CCSPlayerController::FromSlot(slot_) : nullptr;
        if (!layout || !player)
            return;

        layout->SetHasClass("menu_root", kShow, false, player);
        layout->SetHasClass("menu_dim", kShow, false, player);
        layout->SetInputCaptureEnabled(false, player);
    }

    size_t HudMenuInstance::Signature() const
    {
        size_t h = std::hash<std::string>{}(menu_->Title());
        HashIn(h, static_cast<size_t>(currentOffset_));

        const auto& opts = menu_->Options();
        HashIn(h, opts.size());

        const int end = (std::min)(currentOffset_ + kRows, static_cast<int>(opts.size()));
        for (int i = currentOffset_; i < end; ++i)
        {
            HashIn(h, std::hash<std::string>{}(opts[i].Text));
            HashIn(h, opts[i].Disabled ? 1 : 0);
        }

        HashIn(h, std::hash<std::string>{}(hudMenu_->PrevText));
        HashIn(h, std::hash<std::string>{}(hudMenu_->NextText));
        HashIn(h, std::hash<std::string>{}(hudMenu_->CloseText));
        HashIn(h, HasExitButton() ? 1 : 0);
        HashIn(h, hudMenu_->DimBackground ? 1 : 0);
        HashIn(h, EffectiveCapture() ? 1 : 0);
        HashIn(h, static_cast<size_t>(Position()));
        return h;
    }

    void HudMenuInstance::Display()
    {
        if (!player_ || !menu_) return;

        if (g_pToolkitMenus->GetActiveMenu(player_) != this)
        {
            Reset();
            return;
        }

        CCSCustomHudLayout* layout = hud::hudManager.MenuLayout();
        if (!layout)
            return;

        // The evaluators are plugin code and may close this menu or open
        // another one for the player, which destroys this instance.
        const int slot = slot_;
        const uint64_t serial = serial_;

        auto& options = menu_->Options();
        for (size_t i = 0; i < options.size(); ++i)
        {
            auto& opt = options[i];
            if (!opt.DisabledEvaluator) continue;

            opt.Disabled = opt.DisabledEvaluator();
            if (!IsOpen(slot, serial)) return;
        }

        CCSPlayerController* player = player_;
        const auto& opts = menu_->Options();
        const int total = static_cast<int>(opts.size());

        layout->SetDialogVariableString("menu_title", "text", menu_->Title().c_str(), player);

        for (int row = 0; row < kRows; ++row)
        {
            const int idx = currentOffset_ + row;
            if (idx < total)
            {
                const auto& opt = opts[idx];
                layout->SetDialogVariableString(kRowKey[row], "text", std::to_string(row + 1).c_str(), player);
                layout->SetDialogVariableString(kRowText[row], "text", opt.Text.c_str(), player);
                layout->SetHasClass(kRowPanel[row], kDisabled, opt.Disabled, player);
                layout->SetHasClass(kRowPanel[row], kShow, true, player);
            }
            else
            {
                layout->SetHasClass(kRowPanel[row], kShow, false, player);
            }
        }

        layout->SetDialogVariableString("menu_prev_text", "text", hudMenu_->PrevText.c_str(), player);
        layout->SetDialogVariableString("menu_next_text", "text", hudMenu_->NextText.c_str(), player);
        layout->SetDialogVariableString("menu_close_text", "text", hudMenu_->CloseText.c_str(), player);
        layout->SetHasClass("menu_prev", kShow, HasPrevButton(), player);
        layout->SetHasClass("menu_next", kShow, HasNextButton(), player);
        layout->SetHasClass("menu_close", kShow, HasExitButton(), player);

        const int pages = total > 0 ? (total + kRows - 1) / kRows : 1;
        if (pages > 1)
        {
            const std::string footer = std::to_string(page_ + 1) + " / " + std::to_string(pages);
            layout->SetDialogVariableString("menu_footer", "text", footer.c_str(), player);
        }
        layout->SetHasClass("menu_footer", kShow, pages > 1, player);

        layout->SetHasClass("menu_dim", kShow, hudMenu_->DimBackground, player);

        // Where the window sits: all three set, so a position left on the
        // slot by an earlier menu does not linger next to this one's.
        const int pos = Position();
        if (pos != drawnPos_)
        {
            static constexpr const char* kPosClass[] = { "pos-left", "pos-center", "pos-right" };
            for (int i = 0; i < 3; ++i)
                layout->SetHasClass("menu_root", kPosClass[i], i == pos, player);
            drawnPos_ = pos;
        }

        layout->SetHasClass("menu_root", kShow, true, player);
        layout->SetInputCaptureEnabled(EffectiveCapture(), player);

        drawn_ = Signature();
    }

    void HudMenuInstance::OnFrame()
    {
        if (!player_ || !menu_) return;

        const int slot = slot_;
        const uint64_t serial = serial_;

        auto& options = menu_->Options();
        for (size_t i = 0; i < options.size(); ++i)
        {
            auto& opt = options[i];
            if (!opt.DisabledEvaluator) continue;

            opt.Disabled = opt.DisabledEvaluator();
            if (!IsOpen(slot, serial)) return;
        }

        if (Signature() != drawn_)
            Display();
    }

    void HudMenuInstance::OnKeyPress(CCSPlayerController* p, int key)
    {
        if (p != player_) return;
        if (!menu_) return;

        // 7 = Prev, 8 = Next, 9 = Close, as the center HTML menu.
        if (key == 8 && HasNextButton())
        {
            NextPage();
            return;
        }
        if (key == 7 && HasPrevButton())
        {
            PrevPage();
            return;
        }
        if (key == 9 && HasExitButton())
        {
            Close();
            return;
        }

        if (key < 1 || key > kRows) return;

        const int idx = currentOffset_ + (key - 1);
        auto& options = menu_->Options();
        if (idx < 0 || idx >= (int)options.size()) return;

        auto& opt = options[idx];
        if (opt.Disabled || !opt.OnSelect) return;

        // The handler may close this menu or open another one for the player
        // (a submenu), and either destroys this instance. Nothing of `this`
        // may be touched after the call unless it is still the open menu.
        CCSPlayerController* player = player_;
        const int slot = slot_;
        const uint64_t serial = serial_;

        // Called through a copy: a handler that rebuilds this same menu in
        // place (ClearOptions + AddMenuOption) destroys the std::function it
        // is running from.
        const auto onSelect = opt.OnSelect;
        onSelect(player, opt);

        if (!IsOpen(slot, serial))
            return;

        switch (menu_->GetPostSelectAction())
        {
        case PostSelectAction::Close:
            Close();
            return;
        case PostSelectAction::Reset:
            while (!prevPageOffsets_.empty()) prevPageOffsets_.pop();
            page_ = 0;
            currentOffset_ = 0;
            break;
        case PostSelectAction::Nothing:
        default:
            break;
        }

        // Whatever the handler changed is on screen right away; OnFrame()
        // would catch it a frame later anyway.
        Display();
    }

    void HudMenuInstance::Close()
    {
        // CloseActiveMenu destroys this instance; the destructor hides it.
        g_pToolkitMenus->CloseActiveMenu(player_);
    }

    void HudMenuInstance::OnClick(const char* buttonId)
    {
        if (!buttonId || !player_) return;

        int key = 0;
        if (!strncmp(buttonId, "menu_row_", 9))
            key = atoi(buttonId + 9) + 1;
        else if (!strcmp(buttonId, "menu_prev"))
            key = 7;
        else if (!strcmp(buttonId, "menu_next"))
            key = 8;
        else if (!strcmp(buttonId, "menu_close"))
            key = 9;

        if (key < 1 || key > 9)
            return;

        OnKeyPress(player_, key);
    }

    // ---- free functions --------------------------------------------------------

    void Open(PluginId owner, CCSPlayerController* player, HudMenu* menu)
    {
        if (!player || !menu || !g_pToolkitMenus) return;

        // The core takes the instance: it destroys it (Destroy(), so in this
        // library) when the menu closes, is replaced, the player leaves or
        // `owner` unloads.
        g_pToolkitMenus->OpenMenu(owner, player, new HudMenuInstance(player, menu));
    }

    void OnLayoutClick(CCSPlayerController* player, const char* buttonId)
    {
        if (!player) return;

        auto it = s_bySlot.find(player->GetSlot());
        if (it == s_bySlot.end() || !it->second)
            return;

        // Only the menu the core still holds: an instance in the map but no
        // longer active is on its way out.
        if (g_pToolkitMenus->GetActiveMenu(player) != it->second)
            return;

        it->second->OnClick(buttonId);
    }

    void CloseAll()
    {
        // CloseActiveMenu destroys the instance, which erases it from the map.
        std::vector<int> slots;
        slots.reserve(s_bySlot.size());
        for (const auto& kv : s_bySlot)
            slots.push_back(kv.first);

        for (int slot : slots)
        {
            if (CCSPlayerController* player = CCSPlayerController::FromSlot(slot))
                g_pToolkitMenus->CloseActiveMenu(player);
        }

        s_bySlot.clear();
        s_serialBySlot.clear();
    }
}
