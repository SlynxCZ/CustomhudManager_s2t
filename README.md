# CustomhudManager_s2t

The Panorama HUD for [Source2Toolkit](https://www.source2toolkit.net): a toolkit
plugin that serves `ICustomhudManager` -- on-screen texts in eight slots, an
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

The interface is this plugin's, not the core's: `public/ICustomhudManager.h`
is the header (add `public/` to your include path -- a git submodule of this
repository is the usual way), you declare the pointer, and you fetch it once
every plugin is loaded, treating null as "no HUD":

```cpp
#include "ICustomhudManager.h"

ICustomhudManager* g_pCustomhudManager = nullptr;

void MyPlugin::OnAllToolkitPluginsLoaded()
{
    int ret;
    GET_TOOLKIT_IFACE(g_pCustomhudManager, ICustomhudManager, CUSTOMHUD_MANAGER_INTERFACE_VERSION, ret);
}

if (g_pCustomhudManager)
    g_pCustomhudManager->ShowText(player, HudSlot::Top, "Round starts in 5", 4.0f, { HudColor::Yellow, HudSize::Large });
```

A `HudMenu` opens with `g_pCustomhudManager->OpenMenu(g_PluginID, player, &menu)`; the rows are clicked or picked with 1-9,
and a dead player gets the cursor even in the keys mode, since the slot binds
do nothing without a pawn.

`public/ICustomhudManager.h` documents every call and the layout contract; the [website](https://www.source2toolkit.net/docs/panorama/hud) has
the guide.

## Build

Two ways, both against the Source2Toolkit SDK (`SOURCE2TOOLKIT_SDK`). s2sdk,
with the game protobufs, is the SDK's `vendor/s2sdk` submodule unless `S2SDK`
(or `HL2SDKCS2`) points at your own checkout:

```bash
# CMake
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
# AMBuild
mkdir build && cd build && python ../configure.py --enable-optimize --sdks cs2 && ambuild
```

The CI builds with AMBuild on both platforms. A line appended to
`pushbuild.txt` on `main` makes it cut the next tag
and publish the Linux and Windows archives; the plugin reports that tag and
the commit as its version (`toolkit list`).

## License

GPLv3, with the same linking exception as Source2Toolkit. See `LICENSE`.
