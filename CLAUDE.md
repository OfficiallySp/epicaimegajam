# MegaJam26 — 2026 Epic AI MegaJam entry (Unreal Engine 5.8.4, C++ + Blueprints, solo)

Solo entry by Shane, who is new to Unreal. This repo root **is** the Unreal project root: `MegaJam26.uproject`,
the configured `.mcp.json` and this file all live here, and `claude` is launched from here so they load.
Theme: **DIRECTOR’S CUT**. The game is **Fix It In Post** (concept and rules: `docs/design.md`): every level is a
finished movie scene that goes wrong; the player cuts, splices and restores clips on each actor's film strip so the
scene ends as scripted. Everything is C++ (no Blueprints); the set is built procedurally in one empty map, `L_Stage`.

## State of the game (keep this section current; it is the hand-over between sessions)
- `Source/MegaJam26/Sim/`: engine-free beat simulation (`MJSim.cpp`), level text format + embedded campaign
  (`MJLevels.cpp`, `MJLevelData.inl`: 18 scenes in 3 reels; clip lock suffix `!` goes last, e.g. `z2=WATCH!`), brute-force solver (`MJSolver.cpp`).
- `Game/MJDirector`: flow (Title, Edit, Screening, Failed, Printed, Credits), save (`UMJSave`), hints, audio by cue
  name (`/Game/MegaJam26/Audio/SW_<cue>`), `mj.*` console commands for every player action.
- `Game/MJStage`: builds the set from the Blender kit (`/Game/MegaJam26/Kit/SM_<piece>`, materials assigned by slot
  name), Meshy figurines (`Characters/SM_<Look>`) and desk props (`Props/SM_<Name>`); poses everything as a pure
  function of fractional beat time; lights, exposure and tilt-shift depth of field per reel.
- `UI/SMJHud`: the whole UI is one custom-painted Slate leaf widget (immediate-mode buttons, own hit testing).
- Source art, audio and manifests live under `assets/src/`; import with `Content/Python/mj_setup.py`.
- Design levels in a text file and check them without compiling:
  `unreal.MJDevLibrary.solve_levels(text, False)` (editor Python). A scene ships when the theatrical cut fails, the
  minimum equals par and the bonus is reachable. Then add it to `MJLevelData.inl` (Live Coding does NOT reload the
  level list; restart the editor).
- Agent loop from the shell (works even when the session's own unreal-mcp connection is down):
  `node tools/mcp/unreal-call.mjs pie start|stop`, `... console "mj.Play r1s1"`, `... py "@script.py"`,
  `... LiveCodingToolset.LiveCodingToolset CompileLiveCoding`; screenshot = `console "shot showui"` then
  `tools/latest-shot.ps1` (copies it to `.cache/shot.png`). Header changes: `tools/rebuild.ps1` (closes only this
  project's editor, full build with `-NoHotReloadFromIDE`, reopens).
- Claude usage is the bottleneck: about one hour of agent work per five-hour window. Commit and push at the end of
  every block; give bounded chores to sub-agents (Codex for Meshy/Blender/shell work, Sonnet for fal).
- Windows System Restore rolls back `.dll`, `.exe`, `.ini` and `.inl` files inside the repo (13 Nov). After a restore:
  `git status`, `git checkout -- <rolled-back files>`, `tools/rebuild.ps1`.
- Test the packaged build with real input: `tools/drive-game.ps1 -Steps "..."` (click / drag / move / key / gshot /
  expect); `gshot` is an in-engine screenshot (F10) and works with the monitors off, `shot` and `record` need a live
  display. `tools/make-video.ps1` records the 58 s gameplay video (needs a monitor on; it refuses black footage).
- The PC drops the GPU device when it enters Modern Standby (editor dies with DXGI_ERROR_DEVICE_REMOVED): just reopen.
- Open: team name and `butler login` (Shane), human playtest with sound, final video run with a monitor on.

## Dates (GMT; both machines are set to Europe/London)
- Wed 11 Nov 19:00: theme reveal on the kickoff stream. After this: ideation on paper only.
- **Thu 12 Nov 05:00: jam opens. Nothing game-specific (code, assets, design text) exists before this moment.**
- Sun 15 Nov 19:00: planned submission. **Mon 16 Nov 01:59: hard deadline; late is disqualified.**
- Hour plan: `docs/jam-runbook.md`. Prep timeline: `docs/timeline.md`.

## Jam rules that touch every task (full summary: `docs/rules-summary.md`)
- Deliverable: a packaged **Win64 Development** build, zipped as `TeamName_GameName.zip`, **under 1 GB**, uploaded to
  Shane's itch.io page and submitted to the jam. Unpackaged projects are rejected.
- Also required: a 30–60 s gameplay video link, a dev-log link (Glass Box prize), team name, member name + Epic email, game name.
- Everything made **before 12 Nov** that ends up in the entry (this file, `tools/`, `docs/`, skills, prompts) is declared in
  `docs/asset-declarations.md`, together with every Fab, marketplace or AI-generated asset and its license.
- AI usage is logged as it happens in `docs/ai-usage-log.md`; it feeds the declaration and the Glass Box dev log.
- Judging, 1–5 each: Ambition & innovation, Theme, Fun, Gameplay, Visuals, Level design & progression, Audio.
  "Custom gameplay beyond Epic's starter templates is encouraged."

## Machines
- **PC, Framework Desktop (Ryzen AI Max+ 395, Radeon 8060S, 64 GB, Windows 11):** engine, editor, Unreal MCP, packaging.
  Engine: `C:\Users\Shane\Epic Games\UE_5.8`. Project: `C:\Users\Shane\epicaimegajam` (this repo, T3 Code's project folder).
  Compiler: Visual Studio 2026 with the **MSVC 14.50** toolset (5.8's recommended version; 14.51 fails on engine headers).
- **Mac mini (M4, 16 GB):** docs, dev log, scripts, video editing, itch uploads, second Claude window. No engine installed.

## Project conventions
- **C++ first for systems** in `Source/MegaJam26/`: game mode, pawn/player logic, gameplay components, scoring, save state.
  Expose tunables with `UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MegaJam")`; expose hooks with
  `UFUNCTION(BlueprintCallable)` / `BlueprintImplementableEvent`. One module, no plugins of our own.
- **Blueprints for glue and tuning only:** `BP_` subclasses of the C++ classes for mesh, animation, audio and particle
  hookups plus designer values. No core logic in BP graphs; agents cannot read or edit graphs reliably.
- **Editor Python** for bulk work (batch import, renames, property sweeps) in `Content/Python/`; the Python plugin adds that
  folder to `sys.path` and runs `init_unreal.py` on startup (register extra MCP toolsets there).
- **Unreal MCP** for in-editor work: placing and moving actors, materials and instances, lights, PIE runs, Output Log reads.
- Content layout: `Content/MegaJam26/{Maps,Blueprints,Characters,Props,Environment,Materials,VFX,Audio,UI,Input,Data}`.
  Imported packs stay untouched in `Content/External/<PackName>/`; migrate or copy what is used.
- Naming: `L_` map, `BP_` blueprint, `WBP_` widget, `ABP_` anim BP, `M_` material, `MI_` instance, `MF_` material function,
  `T_` texture, `SM_` static mesh, `SK_` skeletal mesh, `A_` animation, `NS_` Niagara system, `SW_` sound wave, `SC_` sound cue,
  `MS_` MetaSound, `IA_` input action, `IMC_` input mapping context, `DA_` data asset, `DT_` data table, `C_` curve.
- Lighting is dynamic (Lumen). `*_BuiltData.uasset` is gitignored; never depend on baked lighting.
- Shipped maps = Project Settings > Packaging > *List of maps to include in a packaged build*. Only real levels go there;
  anything else bloats the zip.

## Agent working rules
1. **One MCP tool call at a time.** The server runs calls serially on the game thread. Never overlap MCP calls and never
   run two MCP clients against one editor.
2. **Verify, do not assume.** After any editor change: save (`File > Save All`, or
   `unreal.EditorAssetLibrary.save_directory('/Game/')`), run PIE for at least 10 s, read the Output Log and quote the
   relevant lines (errors, warnings, `LogTemp` output) in the transcript. A goal is done only with that evidence.
3. **C++ loop:** edit, compile (Live Coding `Ctrl+Alt+F11` while the editor runs; otherwise `Build.bat` below), fix every
   error, PIE, commit. Header changes that add or remove members, `UPROPERTY` or `UFUNCTION` usually need the editor
   closed and a full build. If Live Coding says patching failed or behaviour looks stale: close the editor, full build.
4. **Commit after every green state:** `git add -A && git commit -m "<what works now>"`. Small commits, never leave the
   repo broken overnight, push at least daily.
5. **Always keep a shippable build.** Package with `tools/package.ps1` at least daily and at every milestone in the
   runbook. A feature that breaks packaging is reverted, not debugged at 23:00.
6. **Scope:** definition of done is one mechanic, three progression beats, menus (start, pause, restart, quit), audio on
   every interaction, music. Everything else goes under *Later* in `docs/design.md` and waits.
7. **Blockers:** after 20 minutes on one error, write it in today's devlog under *Blockers*, try the documented fallback,
   move on. Never spend an hour on a problem that does not block shipping.
8. **Log as you go:** every meaningful AI run goes in `docs/ai-usage-log.md` (model, ask, what was kept); every new
   external asset goes in `docs/asset-declarations.md`. Same commit as the change.
9. **Do not touch:** `Saved/`, `Intermediate/`, `Binaries/`, `DerivedDataCache/` (gitignored); renderer and platform
   sections of `Config/DefaultEngine.ini` unless the task is about them; originals under `Content/External/`.

## Commands (PowerShell on the PC, from the repo root; `$UE = 'C:\Users\Shane\Epic Games\UE_5.8'`)
```powershell
# Full editor build (editor closed)
& "$UE\Engine\Build\BatchFiles\Build.bat" MegaJam26Editor Win64 Development -Project="$PWD\MegaJam26.uproject" -WaitMutex
# Regenerate Visual Studio project files
& "$UE\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="$PWD\MegaJam26.uproject" -game -rocket -progress
# Open the editor
& "$UE\Engine\Binaries\Win64\UnrealEditor.exe" "$PWD\MegaJam26.uproject"
# Run an editor Python script headless
& "$UE\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "$PWD\MegaJam26.uproject" -run=pythonscript -script="$PWD\Content\Python\example.py" -unattended -nop4
# Package -> dist\Win64\Windows ; zip -> dist\TeamName_GameName.zip ; upload -> itch channel
.\tools\package.ps1
.\tools\zip-check.ps1
.\tools\upload.ps1
```
Editor console (`~`): `ModelContextProtocol.GenerateClientConfig ClaudeCode` writes `.mcp.json`;
`ModelContextProtocol.RefreshTools` after toolset changes (restart the editor for new C++ tools); `LiveCoding.Compile`.
In Claude Code, `/mcp` shows whether the Unreal server is connected.
The default tool-search mode exposes `list_toolsets`, `describe_toolset`, and `call_tool` for 52 toolsets.
`node tools/mcp/unreal-smoke.mjs` verifies a ten-second PIE session and Output Log access; run it with no other MCP client.
Claude's optional `server/discover` health probe is unsupported by UE 5.8.4 and may log an error before normal initialization;
distinguish that client-probe line from gameplay errors. See `docs/agent-setup.md` for verified setup details.

## Claude Code session defaults (Shane's usage plan)
- Default: Opus 5.5 at `high` (`/model opus`, `/effort high`); `medium` for scripts, dev-log text, batch imports.
- Escalate to Fable 5.2 (`/model claude-fable-5-2`) for concept lock, C++ architecture, packaging or MCP failures and
  bounded overnight goals. Start at `medium`, raise to `high` for hard investigations, and reserve `xhigh` / `max`
  for persistent blockers. One hard turn: put `ultrathink` in the prompt.
- Subagents on Sonnet 5.5 or Haiku 5.5 for verbose chores: import scripts, log triage.
- Work in `/goal` blocks of 60–120 min with a verifiable end state and a turn cap (template below). `/loop` only for
  polling and reminders, never as the build loop.
- Max subscription with usage credits off and no `/fast`. Check `/usage` before heavy days.
- On 11 Nov the Claude subscription is exhausted until the 12 Nov reset: use Codex for prep; do not make Claude
  model calls or switch to API billing to work around the limit. MCP installation/handshake checks need no model calls.
- Project MCP setup and remaining sign-ins: `docs/agent-setup.md`. Use `unreal-mcp` for the editor, `meshy` for 3D,
  `hyper3d-rodin` as the other selected 3D service, and `fal` for images/audio. PBRgen uses web exports.
  Check account/credit status before generation; setup checks must not create assets.

## Goal condition template
```
/goal <Feature> works in PIE: <observable behaviour>.
Done when: (1) the project compiles with 0 errors, (2) PIE runs 30 s with no Error lines in the Output Log,
(3) <specific check, e.g. "pressing E near BP_Door opens it and prints 'Door opened'">,
(4) docs/ai-usage-log.md has a line for this work, (5) the work is committed.
Stop after 25 turns, or after 3 failed attempts at the same error; then write the blocker to devlog/<today>.md and stop.
```

## Key files
`docs/jam-runbook.md` hour plan · `docs/design.md` concept (empty until 12 Nov) · `docs/submission-checklist.md` ·
`docs/pc-bringup.md` machine setup · `docs/learning-plan.md` · `docs/asset-pipeline.md` AI tools to UE ·
`docs/asset-declarations.md` · `docs/ai-usage-log.md` · `devlog/` daily logs mirrored to itch · `tools/README.md` scripts.
