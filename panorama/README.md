# Panorama layouts the core draws with

The core draws HUD menus (`IToolkitMenus::OpenHudMenu`) and HUD texts and
prompts (`IToolkitHud`) through one `custom_hud_layout` entity each. The
layout lives on the client, so it has to reach the players through an addon;
this folder is the reference content of that addon.

```
panorama/layout/custom_game/s2t_menu.xml    menu   -- ids are the contract, keep them
panorama/layout/custom_game/s2t_hud.xml     texts and the interaction prompt
panorama/styles/custom_game/s2t_menu.css    looks -- yours to change
panorama/styles/custom_game/s2t_hud.css
```

The contract (which panel ids the core addresses and which classes it toggles)
is documented at the top of each XML and in `IToolkitHud.h`. Rename the
layouts if you like and put the names into `core.json` (`HudMenuLayout`,
`HudTextLayout`); the ids inside stay.

## Building the addon

1. Install the **Counter-Strike 2 Workshop Tools** (Steam, CS2, DLC).
2. Launch the tools, create an addon (any name, e.g. `myserver_hud`). That
   makes `content/csgo_addons/myserver_hud/` and `game/csgo_addons/myserver_hud/`
   under the CS2 install.
3. Copy the `panorama/` folder of this directory into
   `content/csgo_addons/myserver_hud/`.
4. Compile. Either let the tools do it (Asset Browser picks the files up; the
   Workshop publish step compiles everything), or run the resource compiler
   directly from `game/bin/win64/`:

   ```
   resourcecompiler.exe -i "<cs2>\content\csgo_addons\myserver_hud\panorama\layout\custom_game\s2t_menu.xml"
   resourcecompiler.exe -i "<cs2>\content\csgo_addons\myserver_hud\panorama\layout\custom_game\s2t_hud.xml"
   resourcecompiler.exe -i "<cs2>\content\csgo_addons\myserver_hud\panorama\styles\custom_game\s2t_menu.css"
   resourcecompiler.exe -i "<cs2>\content\csgo_addons\myserver_hud\panorama\styles\custom_game\s2t_hud.css"
   ```

   The compiled `.vxml_c` / `.vcss_c` land under `game/csgo_addons/myserver_hud/`.
5. Publish the addon to the Workshop from the tools (Workshop Manager). Note
   the published file id.
6. Make the server hand the addon to connecting clients. The engine only
   downloads the map's addon by itself; an extra addon needs
   [MultiAddonManager](https://github.com/Source2ZE/MultiAddonManager)
   (`mm_extra_addons <id>`) or an equivalent. Check the server log for
   `S2C_CONNECTION [addons:'<id>']` to see it is being sent.

Panorama caches layouts for the whole client session: after a republish,
players restart the game to see the change.

## Testing without the Workshop

For a local client, a search path in `gameinfo.gi` pointing at the compiled
`game/csgo_addons/myserver_hud/` works too. That is a development setup, not a
way to ship.

## What the layouts must and must not contain

`custom_hud_layout` validates the layout: only `Panel`, `Label`, `Image` and
`Button`; no scripts, no inline `style`, no event attributes; the outermost
panel cannot have an `id`. A layout that violates this loads nothing and the
client console says why. The toolkit docs, "Panorama HUD" > "Authoring
layouts", have the details and the Panorama CSS that works.
