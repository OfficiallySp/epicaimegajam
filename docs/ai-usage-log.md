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
- 2026-11-14 02:10–12:45 GMT · Claude Opus 5.5 (Claude Code in T3 Code) + two Opus review sub-agents + Unreal MCP ·
  Shane asked for a check for glaring issues, then for fixes: solver run over all 18 scenes, PIE screenshots, code
  review. Fixed: pause-menu "rewatch the theatrical cut" no longer wipes the player's edits; hints give back an edit
  first when the budget is spent and treat identical clips as interchangeable; a hint asked for before the solver is
  done no longer stalls the game, and reloading a scene reuses its solve; music volume setting applied once;
  double-click on a button is one press (ROLL no longer rolls and stops); Alt+Enter is fullscreen, not ROLL; F9 and
  PgUp/PgDn need `-MJDev`; edit pips only cross out edits past par · all kept · no human code
- 2026-11-14 12:20–12:50 GMT · Claude Opus 5.5 (Claude Code in T3 Code) + Unreal MCP · Shane's playtest notes: the
  scene auto-playing into an instant CUT!, and the winning-screening camera missing set pieces. Scenes now open in the
  cutting room (reel card, tutorial hint after it, failure reason on the ruler marker; ROLL on the untouched cut is the
  studio's version); coverage is planned from the simulation per beat (master, star close shots, cutaways to
  levers/doors/lasers/knock-outs/the car, zoomed two-shots), checked for all 18 solved scenes with `mj.ShotsAll` and
  PIE screenshots · all kept · no human code
- 2026-11-14 · Codex / GPT-6 Astra in T3 Code · check NVIDIA feature feasibility and prepare OpenAI award disclosure;
  inspect renderer, installed plugins and editor GPU log; verify current jam requirements, OpenAI model name and
  NVIDIA's UE 5.8 DLSS offering · submission copy, credits and checklist updated with Astra's documented set-kit and
  prop-pipeline work, Claude Fable 5.2 / Opus 5.5 and T3 Code orchestration; DLSS recorded as a proposal requiring RTX
  validation, not a shipped feature · documentation only; no renderer changes or RTX testing; time saving not measured
- 2026-11-14 12:42–13:00 GMT · Claude Opus 5.5 (Claude Code in T3 Code) + Unreal MCP · PIE checks of the review
  fixes (rewatch keeps the cut, hint with no edits left, hint before the solve is done, same-scene reload, identical
  clips), team name `officiallysp` in `tools/jam.env` and the itch page draft, final package and zip · all kept
- 2026-11-14 12:45–14:25 GMT · Claude Opus 5.5 (Claude Code in T3 Code) · moved the project to the UE 5.9 MegaJam
  Preview: trial on a worktree copy first, then main (uproject, renamed MCP settings section, tools/jam.env, CLAUDE.md);
  editor target 0 errors, Win64 cook 0 errors/0 warnings, all 18 scenes opened and screened in the packaged build
  (offscreen, console commands) with a screenshot each; new log noise is only ToolsetRegistry "no baked manifest" · all kept
- 2026-11-14 14:25–15:00 GMT · Claude Opus 5.5 (Claude Code in T3 Code) · UE 5.9 web (Wasm) build attempt in the `ue59`
  worktree, following the engine's shipped web-deploy agent skill: emscripten 6.0.10, AI toolsets off for the cook, Development
  build cooks 0 errors and boots in Edge (READY 10.1 s, deferred tier) with a working HUD but a black 3D view; parked at the
  time box. Also: unreal-smoke.mjs moved to ConsoleToolset.LogsToolset (5.9 rename); dev log section · kept on `ue59`
- 2026-11-14 13:00–13:35 GMT · Claude Opus 5.5 (Claude Code in T3 Code) · Shane asked to target as many jam prizes as possible: prize-by-prize eligibility plan (`docs/prize-plan.md`), history scan for secrets before making the repo public, brief for the backdrop art below · all kept · no human code
- 2026-11-14 13:40–17:10 GMT · Codex / GPT-6 Astra (delegated by Claude Opus 5.5) + Meshy `gpt-image-2` · replace the script-painted set backdrops with OpenAI-generated art: 12 wall-flat panels (4 per reel) and a key-art restyle of an in-game frame; `tools/assemble_backdrops.py` crops them to 512x640 and makes every panel edge tile with every other; prompts, task ids and licence in `assets/src/backdrops/README.md` · all kept, no regenerations; 120 Meshy credits (761 → 641)
- 2026-11-14 17:15–17:25 GMT · Claude Opus 5.5 + Unreal MCP (UE 5.9) · reimported the three atlases, PIE screenshots of reels 1, 2 and 3 show the new paintings, posters and lair panels; no errors in the log · all kept
