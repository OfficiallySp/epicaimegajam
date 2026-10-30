---
title: "Prep day 0, Fri 30 Oct: the plan"
itch_post:
---
# Prep day 0, Fri 30 Oct: the plan

**Build:** none (the jam has not started) · **Hours:** ~4 · **Model mix:** Fable 5.1 in Claude Code for research and scaffolding

## Today's goal
Arrive at the jam with a rehearsed pipeline, so that all four jam days go into the game.

## What happened
- Decided the setup: Unreal Engine 5.8.4 on a Framework Desktop (Ryzen AI Max+ 395, 64 GB), C++ project with Blueprint
  subclasses, Claude Code as the single orchestrator using C++, Unreal MCP, editor Python and hand-tuned Blueprints.
  The Mac mini stays the docs, dev-log, video and upload machine.
- Read the jam page: four full days from Thu 12 Nov 05:00 GMT to the Monday 01:59 GMT deadline; packaged development
  build in a `TeamName_GameName.zip` under 1 GB; 30–60 s video; dev log; declare anything made before the jam.
- Scaffolded this repo: `CLAUDE.md` with the agent rules, docs for rules, timeline, PC bring-up, learning plan, asset
  pipeline, runbook, submission checklist, declarations and this dev log; scripts to package, zip-check and upload.
- Installed git-lfs and butler on the Mac; private GitHub repo with LFS for Unreal assets.
- Evening: the PC came online a few days early. Cloned the repo with T3 Code to `C:\Users\Shane\epicaimegajam`
  and a Claude thread on the PC inventoried it: git, GitHub CLI, Claude Code, PowerShell 7, ffmpeg, OBS, Blender and
  the Epic launcher are there; no engine yet and only Visual Studio Build Tools 2026 with MSVC 14.51. Epic's docs say
  5.8 wants MSVC 14.50 and the forums confirm 14.51 breaks engine headers, so the plan moved from VS 2022 to VS 2026
  Community plus the 14.50 toolset. Claude set `core.longpaths`, ran `git lfs install`, installed butler and
  rewrote `docs/pc-bringup.md` with the real paths and findings.

## AI in the loop (Glass Box)
Claude did the research (engine version, MCP setup steps and limits, Claude Code on Windows, the APU's VRAM setting,
the macOS packaging bug and its fix) and wrote the first draft of every doc and script. I set the constraints: solo,
new to Unreal, full-time for the four days, no tool budget limit, Windows build. The scripts have not run on the PC
yet; that is the 3 Nov bring-up.

## Blockers
- None. The PC is online. The Mac fallback is no longer needed.

## Tomorrow
On the PC: install VS 2026 Community with MSVC 14.50 and UE 5.8.4 (admin and launcher steps in `docs/pc-bringup.md`
section 1), `butler login`, then create the Third Person C++ project (section 3). Accounts (Epic 18+, itch, Fab,
GitHub LFS data pack, AI tools), read the Official Rules PDF, watch the MCP setup video and Epic's first-hour and
first-game courses.
