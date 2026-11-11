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
| | | | | |

## D. AI tools used in development
| Tool | Use |
|---|---|
| Claude Code with Claude Opus 5.5, Fable 5.1, Sonnet 5.5, Haiku 5.5 | code, editor automation through Unreal MCP, scripts, planning, dev-log drafts |
| Codex / GPT-6 Astra in T3 Code | pre-jam tooling setup, MCP integration research and configuration (11 Nov) |
| Unreal MCP (Epic, experimental, UE 5.8) | agent access to the editor |
| (add each generator from `docs/asset-pipeline.md` as it is used) | |
