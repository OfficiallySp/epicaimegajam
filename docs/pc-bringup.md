# PC bring-up checklist (Framework Desktop): target Tue 3 Nov, 4–5 h

Work top to bottom, tick the boxes in this file, commit at the end. Keep paths short and outside OneDrive (`C:\Dev\...`).
Optional first step: link the PC in T3 Code so a Claude thread can run the terminal parts with you.

## 0. System
- [ ] Windows 11 fully updated, reboot.
- [ ] AMD Adrenalin, latest, from amd.com (Ryzen AI Max+ 395 / Radeon 8060S). In Adrenalin: Performance > Tuning >
      **Variable Graphics Memory: Custom, about 24 GB**, then reboot. The BIOS UMA buffer is only the boot minimum; leave it.
- [ ] `dxdiag` shows Radeon 8060S, DirectX 12, dedicated memory about 24 GB.
- [ ] At least 150 GB free on the NVMe (`Get-PSDrive C`).
- [ ] Power plan: Best performance. Windows Security > Virus & threat protection > Exclusions: add `C:\Dev` and
      `C:\Program Files\Epic Games` (large compile and cook speed-up).

## 1. Installs (admin PowerShell)
```powershell
winget install -e --id EpicGames.EpicGamesLauncher
winget install -e --id Microsoft.VisualStudio.2022.Community --override "--passive --add Microsoft.VisualStudio.Workload.NativeGame --add Microsoft.VisualStudio.Workload.NativeDesktop --includeRecommended"
winget install -e --id Git.Git
winget install -e --id GitHub.GitLFS
winget install -e --id GitHub.cli
winget install -e --id Anthropic.ClaudeCode        # or: irm https://claude.ai/install.ps1 | iex
winget install -e --id Microsoft.PowerShell          # pwsh 7; the scripts also run on Windows PowerShell 5.1
winget install -e --id OBSProject.OBSStudio
winget install -e --id BlenderFoundation.Blender
winget install -e --id 7zip.7zip
winget install -e --id Gyan.FFmpeg
```
- [ ] butler: https://broth.itch.zone/butler/windows-amd64/LATEST/archive/default, extract to `C:\Tools\butler`,
      add that folder to PATH, then `butler login`.
- [ ] DaVinci Resolve (free) from blackmagicdesign.com (account needed), or CapCut.
- [ ] Epic Games Launcher: sign in, Unreal Engine > Library > **+ > 5.8.4**. Options: keep Starter Content and
      Templates; untick target platforms you will not use (Android, iOS, Linux) to save disk. About 60 GB.
- [ ] Visual Studio: UE 5.8's "Setting Up Visual Studio" docs page lists the exact MSVC and Windows SDK components.
      When the project is generated (step 3) UE writes a `.vsconfig` next to the `.sln`; opening the solution makes the
      VS Installer offer the missing components. Accept. (VS 2022 is the known-good path; check the 5.8 docs before
      trying VS 2026.)
- [ ] Git: `git config --global user.name "Shane"`, `git config --global user.email "<Epic/itch email>"`,
      `git config --global core.longpaths true`, `git lfs install`, `gh auth login`.
- [ ] Claude Code: run `claude` once anywhere and sign in with the Max subscription (not an API key). `claude --version`.

## 2. Clone the repo (repo root = project root)
```powershell
mkdir C:\Dev; cd C:\Dev
git clone https://github.com/OfficiallySp/epicaimegajam.git MegaJam26
cd C:\Dev\MegaJam26
git lfs install
```

## 3. Create the Unreal project inside the repo
The Project Browser always creates `<Location>\<Name>\`, so create it next door and move it in.
- [ ] Launcher > Launch 5.8.4 > Games > **Third Person** > **C++** (not Blueprint); Target platform Desktop;
      Quality Maximum; **Starter Content: off** (keeps the zip small; import what is needed later); Raytracing off.
      Location `C:\Dev\tmp`, name `MegaJam26`. Create. Let it compile and open once, then close the editor.
- [ ] Move it into the repo and build:
```powershell
robocopy C:\Dev\tmp\MegaJam26 C:\Dev\MegaJam26 /E /MOVE /XD Binaries Intermediate Saved DerivedDataCache .vs /XF *.sln
Remove-Item C:\Dev\tmp -Recurse -Force
cd C:\Dev\MegaJam26
$UE = 'C:\Program Files\Epic Games\UE_5.8'
& "$UE\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="C:\Dev\MegaJam26\MegaJam26.uproject" -game -rocket -progress
& "$UE\Engine\Build\BatchFiles\Build.bat" MegaJam26Editor Win64 Development -Project="C:\Dev\MegaJam26\MegaJam26.uproject" -WaitMutex
git status   # expect MegaJam26.uproject, Config/, Content/, Source/, .vsconfig and NO Binaries/Intermediate/Saved
git add -A; git commit -m "Create MegaJam26 (Third Person template, C++)"; git push
```
- [ ] Open the project (double-click the `.uproject`). PIE works with the third-person character. Note the editor
      startup time (shader compile on first open) ___ min.

## 4. Plugins and Unreal MCP
- [ ] Edit > Plugins: enable **Unreal MCP**, **All Toolsets**, **Terminal**, **Python Editor Script Plugin**,
      **Editor Scripting Utilities**. Restart the editor.
- [ ] Editor Preferences > General > **Model Context Protocol** > *Auto Start Server* on (default `http://127.0.0.1:8000/mcp`).
- [ ] Console (`~`): `ModelContextProtocol.GenerateClientConfig ClaudeCode`. `.mcp.json` appears at the repo root.
      `git add .mcp.json; git commit -m "Add Unreal MCP client config"`.
- [ ] From the repo root in a terminal: `claude`, then `/mcp` shows the Unreal server connected. Select an actor in the
      level and ask "Which actors are selected?" Expect the right answer.
- [ ] Terminal plugin: open the in-editor terminal and run `claude` there too; decide which you prefer.

## 5. Smoke tests (write the results in)
- [ ] MCP: "Spawn a 5×5 grid of cubes at 200 cm spacing centred on the origin, then create a red material instance and
      apply it to all of them." Expect cubes in the viewport. Time ___ min, tool calls ___.
- [ ] MCP: "List every toolset and tool you can reach." Fill the **tool inventory** table below.
- [ ] C++: "Add a C++ actor `AJamTestActor` with `UPROPERTY(EditAnywhere) float SpinSpeed` that rotates every tick.
      Compile with Live Coding, place one in the level, run PIE, show the Output Log." Live Coding time ___ s
      (target under 60). Then close the editor, run the full `Build.bat` from step 3, note the time ___ min.
- [ ] Python: "Write `Content/Python/hello.py` that logs the name of every actor in the current level, and run it."
      Record how it ran: an MCP Python tool, the editor Python console, or `UnrealEditor-Cmd -run=pythonscript`.

### Tool inventory
| Toolset | Available? | Notes: what it can and cannot do |
|---|---|---|
| Actors (spawn, move, select, delete, properties) | | |
| Assets (find, import, save, rename, delete) | | |
| Materials and instances | | |
| Blueprint graph authoring | | |
| Blueprint subclass creation and defaults | | |
| UMG widgets | | |
| Niagara | | |
| Landscape / PCG | | |
| Lighting and post-process | | |
| PIE control and Output Log read | | |
| Python execution | | |
| Project settings / config | | |
| Terminal (Claude Code inside the editor) | | |

Limits to remember: calls run serially on the game thread; HTTP/SSE only; editor only (nothing at game runtime); it
cannot author animations; tool search exposes three meta-tools by default; toolset changes need
`ModelContextProtocol.RefreshTools` and new C++ tools need an editor restart. Python functions can be registered as
toolsets with `unreal.ToolsetRegistry.register_toolset_class`: if a needed tool is missing, write it as a Python
toolset in `Content/Python/init_unreal.py` rather than fighting the gap.

## 6. Packaging, zip, upload
- [ ] Project Settings > Packaging: *List of maps to include* = the Third Person map for now; Use Pak File on;
      Use IO Store on; Create compressed cooked packages on; Include Debug Files off. Maps & Modes: Game Default Map set.
- [ ] `.\tools\package.ps1` (first run compiles shaders, 20–40 min; later runs are shorter). Time ___ min.
- [ ] `tools\jam.env`: `JAM_TEAM_NAME=TestTeam` for now, `JAM_GAME_NAME=MegaJam26`. `.\tools\zip-check.ps1` creates
      `dist\TestTeam_MegaJam26.zip` under 1 GB. Size ___ MB.
- [ ] Copy the zip to `C:\Temp\fresh\`, extract, run the exe: the game starts on the Third Person map. (A prerequisites
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

## Fallback if the PC is not online by Wed 4 Nov
Install UE 5.8.4 on the Mac (about 50 GB; an external SSD is fine): Epic Games Launcher for Mac, then 5.8.4; Xcode 27
from the App Store. Package with `tools/package.sh`. The macOS 27 + Xcode 27 "Failed to finalize the .app" bug
(UE-396802) is reported fixed in 5.8.4; if it appears, install Xcode 26.6 side by side and run packaging with
`DEVELOPER_DIR=/Applications/Xcode_26.6.app`. If packaging stops on code signing, set
`[/Script/MacTargetPlatform.XcodeProjectSettings] bUseModernXcode=False` in `Config/DefaultEngine.ini` or configure a
signing team in Project Settings > Platforms > Xcode. The submission would then be a macOS build, which the rules allow.
