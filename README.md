<img src="assets/icon.svg" width="64" height="64" alt="">

# MegaJam26 — 2026 Epic AI MegaJam

Solo Unreal Engine 5.8.4 entry by Shane for the [2026 Epic AI MegaJam](https://itch.io/jam/2026-epic-ai-megajam)
(jam runs Thu 12 Nov 05:00 to Mon 16 Nov 01:59 GMT). This repo is the Unreal project root plus the agent
configuration, build scripts, planning docs and dev log.

**Status: preparation.** The game does not exist until the jam opens. Everything here before 12 Nov is tooling,
and it is declared in `docs/asset-declarations.md`.

## Layout
```
MegaJam26.uproject  Config/  Source/   minimal C++ shell, compiled and MCP-tested on the PC
Content/                            game assets added after the jam opens
.mcp.json           Unreal, Meshy, fal and Hyper3D connections for Claude Code (see docs/agent-setup.md)
CLAUDE.md           rules and commands for Claude Code (run `claude` from this folder)
docs/               rules-summary, timeline, pc-bringup, learning-plan, asset-pipeline,
                    submission-checklist, asset-declarations, ai-usage-log, jam-runbook, design
devlog/             one file per day (+ img/), mirrored to the itch.io dev log
tools/              package.ps1, zip-check.ps1, upload.ps1 and their .sh twins, jam.env
```

## Quick start
- **PC (jam machine):** follow `docs/pc-bringup.md` top to bottom.
- **Open the configured editor:** `.\tools\open-editor.ps1`; then run `claude` here and check `/mcp`.
- **Agent connections (11 Nov update):** see `docs/agent-setup.md` for installed MCPs, account sign-ins and Fable 5.2.
- **Mac (docs, uploads):** `brew install git-lfs`, install [butler](https://itch.io/docs/butler/installing.html),
  `git lfs install`, clone, `butler login`.
- Fill in `tools/jam.env` (team name, game name, itch target) once they are decided.
- Daily: `.\tools\package.ps1` → `.\tools\zip-check.ps1` → test from a fresh folder → `.\tools\upload.ps1`.

## Docs
| File | Purpose |
|---|---|
| `docs/rules-summary.md` | The jam rules that shape the work, prize targets, open questions |
| `docs/timeline.md` | Jam schedule in GMT and the prep schedule Oct 30 to Nov 11 |
| `docs/pc-bringup.md` | Checklist with exact commands to set up the PC, MCP, packaging, upload, video |
| `docs/agent-setup.md` | Current Claude/MCP setup, remaining sign-ins, Fable 5.2 and perk deadlines |
| `docs/partner-mcp-research.md` | Verified partner servers, authentication and web/API credit distinctions |
| `docs/learning-plan.md` | Four hands-on sessions plus timed agent drills |
| `docs/asset-pipeline.md` | One asset through each AI tool into UE, with gotchas |
| `docs/jam-runbook.md` | Hour-by-hour plan for the four jam days and the goal-block pattern |
| `docs/submission-checklist.md` | Sunday: final build, video, itch page, submit |
| `docs/asset-declarations.md` | Pre-jam items, third-party and AI assets, licenses (pasted into the submission) |
| `docs/ai-usage-log.md` | Running log of AI use |
| `docs/design.md` | One-page design, filled at concept lock on 12 Nov |
