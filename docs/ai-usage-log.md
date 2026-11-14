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

- 2026-11-11 · Codex / GPT-6 Astra in T3 Code · record Shane's supplied DIRECTOR’S CUT theme announcement
  in the rules notes, agent instructions and runbook · theme metadata kept; no game concept, gameplay or assets added

## Jam (from Thu 12 Nov 05:00 GMT)

- 2026-11-12 10:30–10:38 GMT · Codex / GPT-6 Astra in T3 Code · test current external tools and MCPs;
  verify four project MCPs and global connections, run a clean ten-second Unreal PIE test, compile/run a C++
  probe, round-trip a Blender GLB and validate audio/ZIP tooling · health report and setup link kept;
  temporary fixtures remain ignored; no paid generation, Claude inference, game-content changes or publication.
  Flagged Unity tool-list timeout, GitHub app reauthentication, PBRgen browser login and itch upload setup;
  Melty's CLI unexpectedly printed its token, which was omitted from repo files and needs rotation.

- 2026-11-12 14:00–14:35 and 19:00–19:15 GMT · Claude Fable 5.2 (Claude Code in T3 Code) + Unreal MCP · brief: "create a
  game for the theme, plan it however you want". Chose the concept (Fix It In Post), wrote `docs/design.md`, then the
  first playable in C++: beat simulation, brute-force solver, ASCII level format, procedural set builder, custom Slate
  timeline UI, console commands for every action, a Python MCP dev toolset, base materials and the stage map via
  editor Python · all kept · first playable loop verified in PIE by Output Log and screenshots; no human code

- 2026-11-13 · Codex / GPT-6 Astra (delegated by Claude Fable 5.2) + Meshy · finish nine props: replace reel/lamp images, create scissors/filmcan images, generate nine GLBs and download existing Baron · all kept; sizes, triangles and bounds checked; 171 credits spent (932 → 761), prior paid tasks excluded · time saving not measured

- 2026-11-13 00:00–00:35 GMT · Claude Sonnet 5.5 subagent + fal · generate the full audio pack: 30 SFX cues and a projector loop (sonilo/v1.1/text-to-sound-effects) plus three instrumental music loops (elevenlabs/music/v2.5), trimmed, normalised and loop-crossfaded with ffmpeg, manifest in assets/src/audio/manifest.md · all kept, no regenerations; many SFX were trimmed to a length cap (generator tails ran longer than targets); est. spend about 2.06 USD of 12 USD; listening checks not done (measured only) · time saving not measured

- 2026-11-13 00:00–00:20 GMT · Codex / GPT-6 Astra (delegated by Claude Fable 5.2) + Blender 5.2 scripting · build a
  20-piece modular set kit as GLBs from one generator script with a re-import check (`tools/blender/make_kit.py`,
  `check_kit.py`) · all 20 pieces kept and imported · replaced the engine-primitive greybox

- 2026-11-13 00:00–00:20, 05:00–05:10 and 08:14–08:40 GMT · Claude Fable 5.2 (Claude Code in T3 Code) + Unreal MCP ·
  game flow (title, theatrical-cut intro, save, hints, pause, scene select, credits), Villain role, audio hooks, 16
  scenes designed against the brute-force solver, kit-based set builder with per-reel looks and tilt-shift depth of
  field, briefs for three sub-agents, import of their output, recovery after a system restore (`git fsck`, LFS check,
  rebuild, shell MCP client) · all kept · no human code
- 2026-11-13 08:40–23:30 GMT · Claude Fable 5.2 (Claude Code in T3 Code) + Unreal MCP · star chimes and projector
  whirr, itch page copy, OFL fonts, close-up coverage camera on winning screenings, F10 in-engine screenshot and
  black-footage guard, clip tooltips and reel cards, laser stubs, hint solver on a worker thread, graphics toggle,
  Reel 3 scenes 5 and 6 (solver-verified), 58 s draft gameplay video, in-engine F8 video recorder (experimental, not
  shipped as a feature) · all kept except the recorder, still under test · no human code
- 2026-11-14 02:00–02:20 GMT · Claude Opus 5.5 (Claude Code in T3 Code) + Unreal MCP · Shane's playtest bug "guards
  face the opposite way": diagnosed from PIE screenshots, one-line fix (`FigureYaw` -90), rebuild, re-check, package;
  licence check of the two fal audio models (both show "Commercial use") · all kept · no human code
- 2026-11-14 11:00–11:35 GMT · Claude Opus 5.5 (Claude Code in T3 Code) · Shane asked for less verbose wording without
  AI-isms: day 3 dev log, plus in-game level notes, hints, tooltips, toasts, failure line, title and credits text · all
  kept · no human code
