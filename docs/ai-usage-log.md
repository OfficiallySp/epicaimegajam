# AI usage log

One line per meaningful AI run. Feeds the declaration (`docs/asset-declarations.md`) and the Glass Box dev log.
Format: `date time GMT · tool/model · what was asked · what was kept · time saved or lost`.

## Before the jam
- 2026-10-30 12:00–16:30 · Claude Fable 5.1 (Claude Code in T3 Code) · research the jam rules, UE 5.8.4, Unreal MCP,
  Claude Code on Windows, the Framework Desktop GPU, the macOS packaging bug; write the readiness plan; scaffold this
  repo (CLAUDE.md, docs, tools, devlog template); install git-lfs and butler on the Mac · all kept · about a day saved
- 2026-10-30 23:10–23:50 · Claude Fable 5.1 (Claude Code in T3 Code, on the PC) · inventory the freshly cloned PC
  against `docs/pc-bringup.md`; check UE 5.8 compiler requirements; set git longpaths and LFS hooks; install butler;
  rewrite the bring-up checklist with the real paths, findings and the VS 2026 / MSVC 14.50 plan · all kept · about
  an hour saved, plus one avoided dead end (MSVC 14.51)

- 2026-11-11 · Codex / GPT-6 Astra in T3 Code · verify official partner MCPs and Fable 5.2, inventory installed tools,
  install pinned Meshy server and register Unreal/Meshy/fal in Claude Code; securely save Meshy key, check MCP handshake,
  start fal OAuth, update setup/runbook/declarations · tooling and documentation kept; no Claude model calls, paid
  generation or game assets; account validation/editor connectivity remain pending · time saving not measured

- 2026-11-11 · Codex / GPT-6 Astra in T3 Code · follow up on Shane's Meshy/fal/Hyper3D/PBRgen accounts;
  verify Meshy's online balance successfully, register official Hyper3D MCP, complete fal/Hyper3D OAuth and
  verify both report Connected; document PBRgen's manual export route · tooling/docs kept; no generation or Claude model calls

- 2026-11-11 · Codex / GPT-6 Astra in T3 Code · locate the existing per-user UE 5.8.4 installation,
  create a minimal C++ setup shell (optional templates absent), enable editor-only Epic MCP/plugins and auto-start,
  correct engine paths, compile and generate VS files, verify Claude connection and 52 toolsets, run ten-second PIE
  and read logs, fix the tooling-related GameFeatureData startup configuration, disable unused Android File Server
  and remove its generated authentication settings from shared config · setup shell, scripts and docs kept;
  no custom game assets or gameplay, no Claude model calls; packaging not yet tested · time saving not measured

## Jam (from Thu 12 Nov 05:00 GMT)
