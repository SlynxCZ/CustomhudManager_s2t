# CustomhudManager_s2t

The Panorama HUD for [Source2Toolkit](https://www.source2toolkit.net): a toolkit
plugin that serves `IToolkitHud` -- on-screen texts in eight slots, an
interaction prompt, toasts, an announcement, a countdown, a big timer, status
chips, a corner card, a progress bar, hit feedback, an event feed, full-screen
overlays -- and `HudMenu`, a menu drawn on the HUD through the core's menu
system. Everything is per player and drawn with the game's own
`custom_hud_layout` entity, from two layouts in a Workshop addon the players
have.

## Install

1. Drop `addons/source2toolkit/plugins/customhud_manager.stx` from a release next to
   the other toolkit plugins. The core loads it with the rest.
2. Compile `panorama/` into an addon (see `panorama/README.md`) and hand it
   to the players -- a Workshop addon with MultiAddonManager, or the map's
   own. Without it a player sees no text and no menu.

## Use from a plugin

The interface is not one of the core's, so `TOOLKIT_SAVEVARS()` leaves
`g_pToolkitHud` null. Fetch it once every plugin is loaded and treat null as
"no HUD":

```cpp
#include "source2toolkit/IToolkitHud.h"

void MyPlugin::OnAllToolkitPluginsLoaded()
{
    int ret;
    GET_TOOLKIT_IFACE(g_pToolkitHud, IToolkitHud, TOOLKIT_HUD_INTERFACE, ret);
}

if (g_pToolkitHud)
    g_pToolkitHud->ShowText(player, HudSlot::Top, "Round starts in 5", 4.0f, { HudColor::Yellow, HudSize::Large });
```

A `HudMenu` opens with `g_pToolkitHud->OpenMenu(g_PluginID, player, &menu)`
(`OPEN_HUD_MENU(player, &menu)`); the rows are clicked or picked with 1-9,
and a dead player gets the cursor even in the keys mode, since the slot binds
do nothing without a pawn.

`source2toolkit/IToolkitHud.h` in the SDK documents every call and the layout
contract; the [website](https://www.source2toolkit.net/docs/panorama/hud) has
the guide.

## Build

Two ways, both against the Source2Toolkit SDK (`SOURCE2TOOLKIT_SDK`), the
hl2sdk (`HL2SDKCS2`) and the game protobufs (`CSGO_PROTO`):

```bash
# CMake
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
# AMBuild
mkdir build && cd build && python ../configure.py --enable-optimize --sdks cs2 && ambuild
```

A line appended to `pushbuild.txt` on `main` makes the CI cut the next tag
and publish the Linux and Windows archives; the plugin reports that tag and
the commit as its version (`toolkit list`).

## License

GPLv3, with the same linking exception as Source2Toolkit. See `LICENSE`.
