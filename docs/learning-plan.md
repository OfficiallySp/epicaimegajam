# Hands-on learning: Wed 4 to Sat 7 Nov, 2–3 h per day, then timed drills

Rules: a separate throwaway project `C:\Dev\Sandbox` (Third Person, C++, its own `.mcp.json` and a copy of `tools/`).
Claude Code is the tutor: start each session with "teach me X in the Sandbox by making me do it; explain each step in two
sentences". **Every session ends with a packaged build** (`.\tools\package.ps1` from the Sandbox copy of tools).
Nothing from the Sandbox is reused in the jam project unless it is declared.

Watch-only before the PC arrives (Mac, ~4 h): Unreal MCP setup video https://www.youtube.com/watch?v=FON_noub0C0,
Unreal MCP livestream https://www.youtube.com/watch?v=k0tgmrBuIJc, Epic's *Your First Hour in Unreal Engine 5* and
*Your First Game in Unreal Engine 5* (Luke Anderton, about 2 h 43 min, Blueprints) at
https://dev.epicgames.com/community/learning, and the packaging doc
https://dev.epicgames.com/documentation/en-us/unreal-engine/packaging-your-project.

## Session 1, Wed 4 Nov: editor and world
Goals: navigate and place things fast; know where the packaging settings live.
- Viewport navigation, snapping, duplicating (Alt-drag), grouping, Outliner folders.
- Lumen on and off (Project Settings > Rendering), what it costs; a directional light + sky atmosphere + exponential height fog; a post-process volume.
- Materials: a master material with three parameters, two instances; apply to meshes.
- Fab: import one free pack and one Megascans surface; where things land in Content.
- Project Settings that matter for packaging: Maps & Modes (default maps), Packaging (maps to cook, pak, IO store, compression), Description (project name shown in the window title).
Done when: a small lit scene with a material instance is packaged and runs. Notes:

## Session 2, Thu 5 Nov: Blueprints, input, UMG
- Actor Blueprint: components, variables, events (BeginPlay, Tick, overlap), timelines, a simple door or pickup.
- Enhanced Input: an Input Mapping Context with two actions, keyboard + gamepad; swapping contexts.
- UMG: HUD widget bound to a C++ variable; main menu map; pause menu (Esc), restart, quit; settings page with fullscreen and volume (Sound Class + Sound Mix).
Done when: menu → level → pause → restart works in a packaged build. Notes:

## Session 3, Fri 6 Nov: C++ through Claude
- Game mode, game state and player state: what lives where; one gameplay component (e.g. health or score) with `UPROPERTY`/`UFUNCTION` exposure and a BP subclass that uses it.
- Live Coding versus full rebuild: which edits need the editor closed; reading a compile error; `UE_LOG`, `check`, `ensure`.
- Delegates and BP events from C++ (`BlueprintAssignable`); a timer; `TSubclassOf` spawning; data assets for tuning.
- Common error triage: missing include, missing module in `Build.cs`, `GENERATED_BODY`, stale `.generated.h`, hot-reload confusion.
Done when: one feature went prompt → compile → PIE → package with Claude doing the typing. Notes:

## Session 4, Sat 7 Nov: audio, feel, drills
- MetaSounds or Sound Cues: an SFX with random pitch, a looping music track with attenuation off, a Sound Class mix.
- Feel: camera shake, hit-stop via time dilation, a Fab Niagara effect on an event, post-process on damage.
- Drills (below), timed.
Done when: the drill table is filled and a packaged build has audio. Notes:

## Timed agent drills (fill in; these numbers set the jam scope)
| Drill | Prompt gist | Target | Actual | Notes |
|---|---|---|---|---|
| MCP level blockout from a paragraph | "Build a greybox arena 40×40 m with three rooms, cover, a start and an exit, lit with one directional light" | 20 min | | |
| Python batch import | "Import these 20 FBX files from `C:\Dev\assets\`, fix scale to cm, generate simple collision, enable Nanite on meshes over 10k tris, put them in Content/External/Test" | 15 min | | |
| C++ feature end to end | "Add a collectible system: component on the character counts pickups, BP_Pickup actor, HUD shows the count, event when all are collected" | 30 min | | |
| Package, zip, upload | `package.ps1` → `zip-check.ps1` → fresh-folder run → `upload.ps1` | 25 min | | |
| Full editor rebuild (editor closed) | `Build.bat MegaJam26Editor` | 5 min | | |
| Live Coding iteration | one `.cpp` change | 1 min | | |

## Reference
Enhanced Input, UMG, MetaSounds, Live Coding, Packaging: https://dev.epicgames.com/documentation/en-us/unreal-engine/
Unreal MCP: https://dev.epicgames.com/documentation/unreal-engine/unreal-mcp-in-unreal-editor
Claude Code: https://code.claude.com/docs/en/setup
