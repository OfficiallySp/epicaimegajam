# Jam runbook: Thu 12 to Sun 15 Nov 2026 (GMT)

Theme reveal Wed 11 Nov 19:00. Jam opens Thu 05:00. Submit Sunday by 19:00. Hard deadline Monday 01:59.
Times below are targets; the milestones in **bold** are the ones that matter. Standing rules are at the end.

## The work block (repeat all day)
1. **Plan, 5 min.** Take the next item from the day list. Write the `/goal` with a verifiable end state and a turn cap.
2. **Run, 60–120 min.** The goal runs in auto mode on the PC. Meanwhile Shane playtests the previous build, writes the
   devlog, generates assets on the Mac, or hand-tunes Blueprints that the agent is not touching.
3. **Verify, 10 min.** Read the transcript's evidence (compile output, Output Log lines, screenshot). Play it.
   Commit if green. `package.ps1` at milestones.
4. **Log, 5 min.** One line in `docs/ai-usage-log.md`, one bullet in today's devlog.

Goal template:
```
/goal <Feature> works in PIE: <observable behaviour>.
Done when: (1) the project compiles with 0 errors, (2) PIE runs 30 s with no Error lines in the Output Log,
(3) <specific check>, (4) docs/ai-usage-log.md has a line for this work, (5) the work is committed.
Stop after 25 turns, or after 3 failed attempts at the same error; then write the blocker to devlog/<today>.md and stop.
```
Overnight goal (optional, Thu and Fri only): `docs/tonight.md` with 3–5 bounded items, run on Fable 5.1 `xhigh`:
"work through docs/tonight.md top to bottom; same done-criteria per item; stop after 40 turns or by 06:30 GMT".

Models: Opus 5.5 `high` by default. Fable 5.1 `xhigh` for concept lock, the C++ architecture pass, packaging or MCP
failures, and overnight goals. Sonnet 5.5 / Haiku 5.5 subagents for import scripts and log triage. One hard turn: `ultrathink`.

## Thu 12 Nov: core
| Time | Milestone |
|---|---|
| 07:00 | Start. Re-read Wednesday's two finalists; pick one. |
| 07:00–09:00 | Fill `docs/design.md` (one page). Fable `xhigh` as sounding board: "name the three biggest risks in this concept and the smallest version that still hits the theme". |
| **09:00** | **Concept lock.** Commit `design.md`. No mechanic changes after this. |
| 09:00–13:00 | Greybox the core mechanic: C++ systems (goal 1), MCP level blockout from the design paragraph (goal 2). Template character as-is. |
| 13:00–14:00 | Lunch, then play the greybox. Fun for 30 s? If not, change numbers, not features. |
| 14:00–19:00 | Playable loop: start, play, win or lose, restart (goal 3). First real level pass (goal 4). SFX hooks in code with placeholder sounds. |
| 19:00–20:00 | Dinner. Daily stream only if ahead. |
| **20:00** | **First packaged build uploaded** to the private channel: `package.ps1`, `zip-check.ps1`, fresh-folder run, `upload.ps1`. **Dev-log #1** posted. |
| 20:30–22:00 | Start AI asset batch #1 on the Mac (per the art and audio plan). Optional overnight goal. |
| 22:00 | Stop. Sleep. |

## Fri 13 Nov: content and progression
| Time | Milestone |
|---|---|
| 07:00–08:00 | Play last night's build. Read the overnight transcript. Top 3 fixes into the day list. |
| 08:00–13:00 | Three progression beats (levels, waves or stages) with a difficulty curve (goals 5–6). Import asset batch #1 via Python script; replace greybox where it is cheap. |
| 13:00–14:00 | Lunch and playtest the three beats end to end. |
| 14:00–19:00 | UI skeleton: main menu, HUD, pause, win/lose screens, restart, quit (goal 7). Enhanced Input remap check (keyboard + gamepad). Asset batch #2 (VFX, UI icons, music loop). |
| **20:00** | **Build #2 uploaded, dev-log #2.** Ask 2–3 remote playtesters to try the private link tomorrow morning. |
| 22:00 | Stop. Optional overnight goal: performance and warnings cleanup. |

## Sat 14 Nov: polish
| Time | Milestone |
|---|---|
| 07:00–08:00 | Play. Playtester notes into the list. |
| 08:00–12:00 | Audio on every interaction + music + attenuation; main menu music; volume settings (goal 8). |
| 12:00–13:00 | Lunch. |
| 13:00–17:00 | Lighting, post-process and VFX pass with MCP (goal 9); camera shake and hit feedback; settings menu (fullscreen, volume). |
| 17:00–19:00 | Fix the top 5 issues from playtesting. Nothing new. |
| **20:00** | **Build #3 uploaded, dev-log #3.** Record 3 minutes of raw gameplay footage as insurance. |
| 22:00 | Stop. No overnight goal; sleep matters more tomorrow. |

## Sun 15 Nov: ship
| Time | Milestone |
|---|---|
| 07:00–09:00 | Play the full game twice. Bug list only. |
| **09:00** | **Content freeze.** Bug fixes only; a fix that needs a feature is not a fix. |
| 09:00–13:00 | Fix bugs. `git tag -f last-good` at every green point. |
| **13:00** | **Final package**: `package.ps1 -Clean`, `zip-check.ps1`, fresh-folder run on the PC, copy the zip to the Mac. |
| 14:00–16:00 | Record and cut the 30–60 s video from the packaged build; upload to YouTube; check the link. |
| 16:00–18:00 | itch page: description, controls, screenshots, cover, declarations (pre-made items, Fab assets, AI tools), final dev-log. Upload the zip via the web dashboard. **Submit to the jam.** Confirm the entry appears. |
| 19:00 | Done. The 01:59 Monday deadline is a buffer, not a plan. |

Full Sunday list: `docs/submission-checklist.md`.

## If behind
- Fri 19:00 without three beats: ship two beats, cut the third, say so in the devlog.
- Sat 12:00 without audio: ElevenLabs batch of 10 SFX plus one Suno loop, wired by a Python script; skip the VFX pass.
- Sun 13:00 package fails: `git checkout last-good`, package that. A worse game that ships beats a better one that does not.
- Any time MCP is flaky: fall back to C++ and Python scripts; restart the editor once; do not debug MCP during the jam.

## Standing rules
- Always have a shippable build. Commit after every working increment. Push daily.
- One MCP client at a time, one MCP call at a time.
- Log AI usage as you go (`docs/ai-usage-log.md`); add every external asset to `docs/asset-declarations.md` in the same commit.
- Concept lock Thu 09:00, content freeze Sun 09:00. Definition of done: one mechanic, three beats, menus, audio.
- Sleep is scheduled. Start days at a normal wake time.
- Go light on Claude usage on 9–11 Nov; check `/usage` for the weekly reset on 11 Nov and plan the heavy days around it.

## Links
Jam page and submission: https://itch.io/jam/2026-epic-ai-megajam · Epic forums thread: https://epic.gm/2026-ue-megajam ·
itch dashboard: https://itch.io/dashboard · YouTube upload: https://studio.youtube.com · Unreal MCP docs:
https://dev.epicgames.com/documentation/unreal-engine/unreal-mcp-in-unreal-editor · packaging:
https://dev.epicgames.com/documentation/en-us/unreal-engine/packaging-your-project
