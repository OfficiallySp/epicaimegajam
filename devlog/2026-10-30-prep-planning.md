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

## AI in the loop (Glass Box)
Claude did the research (engine version, MCP setup steps and limits, Claude Code on Windows, the APU's VRAM setting,
the macOS packaging bug and its fix) and wrote the first draft of every doc and script. I set the constraints: solo,
new to Unreal, full-time for the four days, no tool budget limit, Windows build. The scripts have not run on the PC
yet; that is the 3 Nov bring-up.

## Blockers
- The PC is offline until early next week. If it slips past 4 Nov, the fallback is UE on the Mac with a macOS build.

## Tomorrow
Accounts (Epic 18+, itch, Fab, GitHub LFS data pack, AI tools), read the Official Rules PDF, watch the MCP setup video
and Epic's first-hour and first-game courses.
