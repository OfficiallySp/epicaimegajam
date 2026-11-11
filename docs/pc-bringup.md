# PC bring-up checklist (Framework Desktop): updated 11 Nov 2026

**11 Nov update:** UE 5.8.4 is installed at `C:\Users\Shane\Epic Games\UE_5.8`. The minimal C++ project
compiled in 78 seconds using MSVC 14.50, and Unreal MCP connects in Claude with 52 discoverable toolsets.
A ten-second PIE check passed with zero Error/Fatal lines and zero warnings. VS Community 2026 and 7-Zip
are installed; Meshy, fal and Hyper3D are connected. Optional project templates are not installed.
See `docs/agent-setup.md` for details. Packaging/upload and the optional learning drills below remain unchecked.

Work top to bottom, tick the boxes in this file, commit at the end. The repo is at `C:\Users\Shane\epicaimegajam`
(T3 Code's project folder: short path, outside OneDrive); the engine is at `C:\Users\Shane\Epic Games\UE_5.8`.
The PC arrived early. On 30 Oct a Claude thread on the PC ran the checks below and filled in what it found.

## 0. System
- [x] Windows 11 Pro build 26300 (30 Oct). Run Windows Update once more and reboot before the engine install.
- [ ] AMD Adrenalin: driver 32.0.32015.7029 dated 18 Oct 2026 is installed. Windows sees 31.8 GB of RAM, so the GPU
      already holds 32 GB of Variable Graphics Memory. That works for UE; 24 GB leaves more for compiles and cooks, so
      set Performance > Tuning > Variable Graphics Memory to Custom, 24 GB if the editor or a cook runs out of system
      memory. The BIOS UMA buffer is only the boot minimum; leave it.
- [ ] `dxdiag` shows Radeon 8060S, DirectX 12, dedicated memory 24–32 GB.
- [x] Disk (30 Oct): C: 512 GB free, D: 137 GB free. Engine (60 GB), DDC and builds all fit on C:.
- [ ] Power mode: Settings > System > Power > Power mode: **Best performance** (`powercfg` cannot set this overlay on
      this build and Balanced is the only classic scheme). Windows Security > Virus & threat protection > Exclusions:
      add `C:\Users\Shane\epicaimegajam`, `C:\Users\Shane\Sandbox` and `C:\Program Files\Epic Games`, or in an admin
      PowerShell:
      `Add-MpPreference -ExclusionPath 'C:\Users\Shane\epicaimegajam','C:\Users\Shane\Sandbox','C:\Program Files\Epic Games'`

## 1. Installs
Found on 30 Oct: Git 2.55 with LFS 3.7.1, GitHub CLI 2.103 (logged in as OfficiallySp), Claude Code 2.1.308 (logged in
with the claude.ai account), PowerShell 7.6, ffmpeg 9.0.1, OBS Studio, Blender, Python 3.12, Epic Games Launcher (user
install), Visual Studio **Build Tools 2026** with MSVC 14.51 and Windows SDK 10.0.26100, .NET 10 runtime.
Not found: any Unreal Engine, a Visual Studio IDE, MSVC 14.50, 7-Zip, butler (installed below).

Compiler facts for UE 5.8 (Epic docs and forums, 30 Oct): Visual Studio 2026 18.0+ is the recommended IDE (2022 17.14+
is the minimum); MSVC minimum 14.38, **recommended 14.50**; Windows SDK 10.0.26100; .NET 10. MSVC **14.51 fails** to
compile engine code in 5.8 (`nvtess.h` includes `<hash_map>`); Epic's answer is "go back to 14.50", and
UnrealBuildTool picks 14.50 by itself once it is installed (`Engine\Config\Windows\Windows_SDK.json`). So: install
VS 2026 Community with the 14.50 toolset component and keep Build Tools 2026 (harmless).

- [x] Visual Studio 2026 Community with MSVC 14.50 (verified 11 Nov). Original install instructions, admin PowerShell:
```powershell
winget install -e --id Microsoft.VisualStudio.Community --override "--passive --norestart --add Microsoft.VisualStudio.Workload.NativeDesktop --add Microsoft.VisualStudio.Workload.NativeGame --add Microsoft.VisualStudio.Workload.ManagedDesktop --add Microsoft.VisualStudio.Component.VC.14.50.18.0.x86.x64 --includeRecommended"
```
      or through the VS Installer that Build Tools 2026 already put on the machine (same channel, `VisualStudio.18.Release`):
```powershell
& 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\setup.exe' install --channelId VisualStudio.18.Release --productId Microsoft.VisualStudio.Product.Community --add Microsoft.VisualStudio.Workload.NativeDesktop --add Microsoft.VisualStudio.Workload.NativeGame --add Microsoft.VisualStudio.Workload.ManagedDesktop --add Microsoft.VisualStudio.Component.VC.14.50.18.0.x86.x64 --includeRecommended --passive --norestart
```
      Then check that `Get-ChildItem 'C:\Program Files (x86)\Microsoft Visual Studio\18\Community\VC\Tools\MSVC'`
      lists `14.50.*`. When UE writes a `.vsconfig` next to the `.sln` (step 3), accept whatever components it asks for.
- [x] 7-Zip (verified 11 Nov at `C:\Program Files\7-Zip`; faster zips that skip `*.pdb`), original install command:
      `winget install -e --id 7zip.7zip`
- [x] butler 15.32.0 extracted to `C:\Users\Shane\.local\bin` (already on the user PATH, next to `claude.exe`).
- [ ] `butler login` (opens a browser; the token lands in `%APPDATA%\itch`).
- [ ] DaVinci Resolve (free) from blackmagicdesign.com (account needed), or CapCut.
- [x] UE **5.8.4** found in the per-user installation at `C:\Users\Shane\Epic Games\UE_5.8` and launched.
      Optional templates/feature packs are absent. Add them through the launcher's engine options if needed later.
- [x] Git: `user.name` Shane, `core.longpaths true`, `git lfs install` run in the repo (hooks present), `gh auth login`
      done. `user.email` is the me.com address; the jam needs the Epic and itch accounts to match, not commit authors.
- [x] Claude Code signed in with the claude.ai account (`claude auth status`).

## 2. Clone the repo (repo root = project root)
- [x] Cloned by T3 Code on 30 Oct to `C:\Users\Shane\epicaimegajam`; `git lfs install` run inside it; `origin` is
      `https://github.com/OfficiallySp/epicaimegajam.git`, branch `main`. The folder name differs from the project
      name on purpose: Unreal only cares about `MegaJam26.uproject` inside it.

## 3. Create the Unreal project inside the repo
- [x] Created `MegaJam26.uproject`, a minimal primary C++ module and game/editor build targets directly in this repo.
      This replaces the originally planned Third Person shell because the installation has no optional templates.
      There is no custom game logic, character, input setup, or saved map yet.
- [x] Full editor build and Visual Studio project generation passed:
```powershell
cd C:\Users\Shane\epicaimegajam
$UE = 'C:\Users\Shane\Epic Games\UE_5.8'
& "$UE\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="$PWD\MegaJam26.uproject" -game -rocket -progress
# Close the editor before a full rebuild.
& "$UE\Engine\Build\BatchFiles\Build.bat" MegaJam26Editor Win64 Development -Project="$PWD\MegaJam26.uproject" -WaitMutex
.\tools\open-editor.ps1
```
- [x] Editor opens the engine's basic template scene as an unsaved level. PIE starts and stops through MCP.
- [ ] If the game needs Epic's Third Person character, install the optional template/feature pack and import it
      deliberately after kickoff. Do not overwrite the existing project or its MCP configuration.

## 4. Plugins and Unreal MCP
- [x] Enabled **Unreal MCP**, **All Toolsets**, **Terminal**, **Python Editor Script Plugin**, and
      **Editor Scripting Utilities** in `.uproject`, limited to editor targets.
- [x] Auto-start configured in `Config/DefaultEditorPerProjectUserSettings.ini`; log confirms `127.0.0.1:8000`.
- [x] `.mcp.json` and this PC's Claude local registration already contain `unreal-mcp` and the three asset services.
      `ModelContextProtocol.GenerateClientConfig ClaudeCode` can refresh the project entry if required.
- [x] `claude mcp get unreal-mcp` reports **Connected**. Direct MCP actor-selection and Output Log reads passed
      without a Claude model call. Use `/mcp` when resuming Claude after the allowance resets.
- [ ] Terminal plugin: open the in-editor terminal and run `claude` there too; decide which you prefer.

## 5. Smoke tests (write the results in)
- [x] `node tools/mcp/unreal-smoke.mjs`: selection query; confirm PIE off; run ten seconds; confirm PIE on;
      stop; confirm PIE off; read `LogPlayLevel`. Zero Error/Fatal lines and zero warnings during the test.
      Report: ignored `.cache/unreal-smoke.json`. Do not run another MCP client during this check.
- [ ] MCP: "Spawn a 5×5 grid of cubes at 200 cm spacing centred on the origin, then create a red material instance and
      apply it to all of them." Expect cubes in the viewport. Time ___ min, tool calls ___.
- [x] MCP `list_toolsets`: **52 toolsets** discovered. Table below distinguishes discovery from exercised functions.
- [ ] C++: "Add a C++ actor `AJamTestActor` with `UPROPERTY(EditAnywhere) float SpinSpeed` that rotates every tick.
      Compile with Live Coding, place one in the level, run PIE, show the Output Log." Live Coding time ___ s
      (target under 60). Then close the editor, run the full `Build.bat` from step 3, note the time ___ min.
- [ ] Python: "Write `Content/Python/hello.py` that logs the name of every actor in the current level, and run it."
      Record how it ran: an MCP Python tool, the editor Python console, or `UnrealEditor-Cmd -run=pythonscript`.

### Tool inventory
| Toolset | Available? | Notes: what it can and cannot do |
|---|---|---|
| Actors (spawn, move, select, delete, properties) | Discovered | ActorTools, SceneTools, ObjectTools; selected-actor read tested |
| Assets (find, import, save, rename, delete) | Discovered | AssetTools; imports not yet exercised |
| Materials and instances | Discovered | MaterialTools, MaterialInstanceTools |
| Blueprint graph authoring | Discovered | BlueprintTools; workflows not yet exercised |
| Blueprint subclass creation and defaults | Discovered | BlueprintTools and ObjectTools |
| UMG widgets | Discovered | UMGToolSet |
| Niagara | Discovered | Info, Component, Blueprint, System and Assets toolsets |
| Landscape / PCG | Partial discovery | PCGToolset and PCGSpatialToolset; dedicated landscape tools not verified |
| Lighting and post-process | General tools | Actor/Object/Scene property tooling; workflow not tested |
| PIE control and Output Log read | Tested | StartPIE, IsPIERunning, StopPIE, GetLogEntries |
| Python execution | Limited MCP scripting | ProgrammaticToolset is sandboxed tool orchestration, not arbitrary Python. Python Editor Script Plugin is enabled for normal editor scripts |
| Project settings / config | Discovered | ConfigSettingsToolset |
| Terminal (Claude Code inside the editor) | Enabled | Plugin loaded; in-editor terminal workflow not yet tried |

Limits to remember: calls run serially on the game thread; HTTP/SSE only; editor only (nothing at game runtime); it
includes animation/Sequencer toolsets whose workflows still need validation; tool search exposes three meta-tools by default; toolset changes need
`ModelContextProtocol.RefreshTools` and new C++ tools need an editor restart. Python functions can be registered as
toolsets with `unreal.ToolsetRegistry.register_toolset_class`: if a needed tool is missing, write it as a Python
toolset in `Content/Python/init_unreal.py` rather than fighting the gap.

## 6. Packaging, zip, upload
- [ ] After creating the game's first saved map, Project Settings > Packaging: *List of maps to include* = that map; Use Pak File on;
      Use IO Store on; Create compressed cooked packages on; Include Debug Files off. Maps & Modes: Game Default Map set.
- [ ] `.\tools\package.ps1` (first run compiles shaders, 20–40 min; later runs are shorter). Time ___ min.
- [ ] `tools\jam.env`: `JAM_TEAM_NAME=TestTeam` for now, `JAM_GAME_NAME=MegaJam26`. `.\tools\zip-check.ps1` creates
      `dist\TestTeam_MegaJam26.zip` under 1 GB. Size ___ MB.
- [ ] Copy the zip to `C:\Temp\fresh\`, extract, run the exe: the game starts on the intended map. (A prerequisites
      installer prompt on first run is expected; that is `-prereqs`.)
- [ ] itch.io: create a project (visibility Draft; kind Downloadable; platform Windows), for example
      `https://<user>.itch.io/megajam26`. Put `ITCH_TARGET=<user>/megajam26` in `tools\jam.env`. `butler login`.
      `.\tools\upload.ps1`. On the itch page: download, extract into a fresh folder, run. Downloaded filename ___.
- [ ] Run `.\tools\upload.ps1` again without changes to see the incremental push time ___.

## 7. Video pipeline
- [ ] OBS: 1920×1080, 60 fps, Game Capture of the packaged exe, desktop audio on, mic off. Record 30 s.
- [ ] `ffmpeg -i in.mkv -c:v libx264 -preset slow -crf 18 -pix_fmt yuv420p -c:a aac -b:a 192k -movflags +faststart out.mp4`
- [ ] Upload unlisted to YouTube; open the link on the Mac.

## 8. Wrap-up
- [ ] Commit everything, push. On the Mac: `git pull`, `git lfs ls-files` shows the assets.
- [ ] Write `devlog/2026-11-03-prep.md` from `devlog/_template.md`: what was set up, timings, surprises.
- [ ] Add Claude Code skills in `.claude/skills/` (`package`, `devlog`, `submit-check`) that wrap the scripts and
      checklists, with `effort: low` in their frontmatter. Commit.
- [ ] Put today's timings into `docs/learning-plan.md` (baselines).

## Fallback if the PC is not online by Wed 4 Nov (not needed: the PC came online on 30 Oct)
Install UE 5.8.4 on the Mac (about 50 GB; an external SSD is fine): Epic Games Launcher for Mac, then 5.8.4; Xcode 27
from the App Store. Package with `tools/package.sh`. The macOS 27 + Xcode 27 "Failed to finalize the .app" bug
(UE-396802) is reported fixed in 5.8.4; if it appears, install Xcode 26.6 side by side and run packaging with
`DEVELOPER_DIR=/Applications/Xcode_26.6.app`. If packaging stops on code signing, set
`[/Script/MacTargetPlatform.XcodeProjectSettings] bUseModernXcode=False` in `Config/DefaultEngine.ini` or configure a
signing team in Project Settings > Platforms > Xcode. The submission would then be a macOS build, which the rules allow.
