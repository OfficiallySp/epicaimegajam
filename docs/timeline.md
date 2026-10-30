# Timeline (all times GMT, Europe/London)

UK clocks went back on 25 Oct and US clocks go back on 1 Nov, so every jam event below is PST = GMT−8, EST = GMT−5.

## Jam schedule
| Local (GMT) | Official | Event |
|---|---|---|
| Wed 11 Nov 19:00 | 11 AM PST | Kickoff livestream and **theme reveal** (Twitch / YouTube / X: UnrealEngine) |
| **Thu 12 Nov 05:00** | 12:00 AM EST | **Jam opens.** No building before this. |
| Thu 12 – Sun 15 Nov, 19:00 daily | 11 AM PST | Daily livestreams (watch only if ahead of schedule) |
| Sun 15 Nov 19:00 | | Our planned submission time |
| **Mon 16 Nov 01:59** | Nov 15, 5:59 PM PST | **Submission deadline. Late = disqualified.** |
| by Fri 29 Jan 2027 | | Winners announced |

Four full working days Thu–Sun; the Monday small-hours deadline is an emergency buffer only.

## Prep schedule
| Date | Block | What | Done when |
|---|---|---|---|
| Fri 30 Oct | A | Readiness plan; repo scaffolded (CLAUDE.md, docs, tools, devlog template); Mac tools (git-lfs, butler); private GitHub repo | repo pushed |
| Sat 31 Oct – Sun 1 Nov | A | Accounts (~1 h): Epic 18+, itch join jam, Official Rules PDF, Fab linked, GitHub LFS data pack, Epic forums, AI tools (Tripo, Meshy, Rodin, ElevenLabs, Suno/Udio, image model, Blockade Labs). Watch-only learning (~4 h): MCP setup video + livestream, *Your First Hour in UE5*, *Your First Game in UE5*, packaging doc | `docs/rules-summary.md` open items ticked |
| Mon 2 Nov | buffer | Remaining accounts; team name decided; PC arrives? | |
| **Tue 3 Nov** | B | **PC bring-up** (4–5 h): drivers, installs, UE 5.8.4, VS 2022, repo clone, project creation, MCP, smoke tests, package, zip, upload, video, devlog | every box in `docs/pc-bringup.md` |
| Wed 4 – Sat 7 Nov | C | Hands-on learning, 2–3 h/day in a throwaway Sandbox project; each session ends with a packaged build; timed agent drills on Sat | `docs/learning-plan.md` times filled |
| Sun 8 Nov | D | Asset pipeline tests (~4 h): one asset per AI tool into UE; Fab shortlist saved | `docs/asset-pipeline.md` filled |
| Mon 9 Nov | E | Rehearsal mini-jam, 5 h hard stop: random theme, compressed runbook, zip on the draft page + 30 s video + devlog post; retro into scripts and runbook | retro applied |
| Tue 10 Nov | F | Buffer: leftovers, hidden itch page with placeholders, OBS scenes, dev-log hub; readiness checklist below; go light on Claude usage | all boxes ticked |
| Wed 11 Nov | F | Claim partner credits if announced; sleep early. **19:00 theme reveal, then ideation only**: 5 concepts, 2 finalists, paper sketches, theme-link test. No assets, no code | 2 finalists on paper |
| Thu 12 – Sun 15 Nov | jam | `docs/jam-runbook.md` | submitted Sun 19:00 |

## Readiness checklist (all true by Tue 10 Nov)
- [ ] PC opens `MegaJam26` in UE 5.8.4; Claude Code connects to Unreal MCP and passes the spawn/material smoke test; toolset inventory recorded in `docs/pc-bringup.md`.
- [ ] A C++ change compiles via Live Coding in under a minute; the full-rebuild procedure is documented.
- [ ] `tools/package.ps1` makes a Win64 Development build; `zip-check.ps1` passes (< 1 GB, correct name); the game runs from a fresh folder.
- [ ] `upload.ps1` pushes to a draft itch page; download verified. A 30 s clip went OBS → ffmpeg → YouTube and the link works.
- [ ] One asset from each AI tool is in UE at the right scale with materials; steps written in `docs/asset-pipeline.md`.
- [ ] Rehearsal mini-jam shipped end-to-end in 5 h or less; retro applied to scripts and runbook.
- [ ] Accounts done (Epic 18+, itch joined, Fab, GitHub LFS, AI tools, forums); Official Rules read; partner credits claimed if offered; team name chosen and in `tools/jam.env`.
- [ ] Mac: git-lfs and butler installed; repo pushed; docs and dev log editable from either machine.
