# Declarations (pasted into the itch submission on Sun 15 Nov)

The jam requires declaring (A) anything made before 12 Nov that is used in the entry, (B) all non-original assets with
their license, and (C) the AI tools used. Keep this file current in the same commit as each change.

## A. Made before the jam opened (12 Nov 2026 05:00 GMT)
| Item | What it is | Made on | How it is used in the entry |
|---|---|---|---|
| Repo scaffolding: `CLAUDE.md`, `tools/*.ps1` and `*.sh`, `docs/*`, `devlog/_template.md` | Agent rules, packaging/zip/upload scripts, checklists, runbook, templates | 30 Oct 2026, with Claude (Fable 5.1) in Claude Code | Process only; contains no game content |
| `.mcp.json`, `tools/mcp/` manifests and diagnostic, agent setup and partner MCP notes, runbook updates | Official Unreal/Meshy/fal/Hyper3D connections and setup instructions; no generated game assets | 11 Nov 2026, with Codex / GPT-6 Astra | Development tooling only; Meshy MCP server is MIT-licensed; local credentials excluded |
| Claude Code skills (`/package`, `/devlog`, `/submit-check`) | Wrappers around the scripts and checklists | planned 3 Nov 2026 | Process only |
| `MegaJam26` project shell, `tools/open-editor.ps1`, `tools/mcp/unreal-smoke.mjs` | Minimal C++ module and editor/game targets, editor-only MCP plugins, auto-start configuration, launch and smoke-check helpers | 11 Nov 2026, with Codex / GPT-6 Astra | Development scaffolding only; no gameplay implementation or custom assets. Optional Third Person templates were not installed |
| (add: Python editor scripts from the learning week, prompt templates, OBS scenes, itch page placeholders) | | | |

## B. Third-party assets (Fab, Epic samples, other)
| Asset | Source / URL | License | Where used |
|---|---|---|---|
| Engine basic template scene (setup test only, unsaved) | Epic Games, ships with UE 5.8.4 | UE EULA | Used only for editor/PIE validation; not yet included in a game build |
| | | | |

## C. AI-generated assets
| Asset | Tool and model | Prompt gist | Terms checked (date) | Where used |
|---|---|---|---|---|
| All sound effects and the projector loop | fal, sonilo/v1.1/text-to-sound-effects | Short foley cues for a film-editor desk puzzle (clicks, film rustle, scissors, whooshes, toy-on-wood knocks, brass stab); prompts in assets/src/audio/manifest.md | Commercial-use claim in fal catalogue description only; model page licence not verified (13 Nov 2026) | all game audio (see assets/src/audio/manifest.md)|
| Three music tracks (edit, screening, title) | fal, elevenlabs/music/v2.5 (instrumental) | 1960s spy-caper heist jazz, about 100 bpm, D minor; prompts in assets/src/audio/manifest.md | fal FAQ read; model-specific licence not verified (13 Nov 2026) | all game audio (see assets/src/audio/manifest.md)|

| Nine props: camera, car, diamond, clapper, reel, lamp, mug, scissors, filmcan | Meshy (nano-banana-pro image, meshy-t2.5 + meshy-7 texture) | Toy set props and photoreal film-editor desk objects; prompts in assets/src/props/README.md | not yet checked | desk dressing and set props |
| Baron villain figurine | Meshy (nano-banana-pro image, meshy-t2.5 + meshy-7 texture) | Existing villain toy figurine downloaded without regeneration | not yet checked | desk dressing and set props |

## D. AI tools used in development
| Tool | Use |
|---|---|
| Claude Code with Claude Opus 5.5, Fable 5.1, Sonnet 5.5, Haiku 5.5 | code, editor automation through Unreal MCP, scripts, planning, dev-log drafts |
| Codex / GPT-6 Astra in T3 Code | pre-jam tooling setup, MCP integration research and configuration (11 Nov) |
| Unreal MCP (Epic, experimental, UE 5.8) | agent access to the editor |
| Claude Fable 5.2 (Claude Code in T3 Code) | concept, design, all game code, level design with a solver, editor automation, sub-agent briefs, dev log (12 Nov onward) |
| Claude Sonnet 5.5 (sub-agent) | ran the fal audio generation and post-processing |
| Codex / GPT-6 Astra (sub-agents, 13 Nov) | ran Meshy prop generation; wrote the Blender script that builds the set kit |
| Meshy (nano-banana-pro images, meshy-t2.5 geometry, meshy-7 textures) | figurines and props |
| fal (sonilo/v1.1 sound effects, elevenlabs/music/v2.5) | all sound effects and music |
| Blender 5.2 (scripted, headless) | modular set kit |
