# Prize plan (14 Nov 2026, 13:00 GMT)

Goal: give every prize this entry can win a real shot, without risking the one thing every prize needs, a valid
submission by Sun 15 Nov 19:00 GMT (hard deadline Mon 16 Nov 01:59).

## What we can and cannot enter
| Prize | Eligible? | Status | What moves the needle |
|---|---|---|---|
| Best Overall UE (+ Continuation Grant) | yes | entered | the 7 judging criteria; the video and first 3 minutes decide most of it |
| Best Overall UE: OpenAI Award | yes | qualifies (GPT-6 Astra via Codex built the set kit and prop pipeline) | make OpenAI's part visible on screen, not only in tooling (see A2) |
| Solo Driver | yes, automatic | entered | same as Best Overall |
| Glass Box (dev log) | yes | logs exist in `devlog/`, not yet published | publish them, plus one strong "how it was built" post (A1) |
| Render Royalty | yes | entered on visuals | the edit view is flat (purple blocks); closeups already look good (A3) |
| Render Royalty: OpenAI Award | yes | qualifies, weakly | OpenAI-made art in the frame (A2) |
| Render Royalty: NVIDIA RTX Award | only with DLSS / NvRTX / ACE | not qualifying | GPU goes to the **highest-ranked Render Royalty entry that qualifies**, not only the winner; few entries will qualify, so a decent entry with DLSS has a real chance (B1) |
| Rodin cash pool ($10k split) | only if Rodin is used and listed | not used | one Rodin-made prop in the game, list Rodin in "Tools you used" (A4) |
| Meshy / Tripo all-category prizes | automatic for any winner | n/a | nothing to do |
| PBRgen (Render Royalty partner) | automatic for that winner | n/a | optional: one PBRgen material (desk wood) |
| Best Student | no (Shane is not enrolled) | n/a | n/a |
| Pocket Powerhouse (+ OpenAI) | no | skip | mobile build: Android toolchain, touch UI, perf; would threaten the main submission |
| Best Team, Best UEFN, Walkie-Talkie | no | n/a | solo / UEFN-only |

## A. Do (no-regret, in order)
1. **Glass Box.** Publish day logs 12, 13, 14 and 15 Nov as itch devlog posts (with images), plus a final post:
   timeline, numbers (commits, C++ lines, agent hours, credits spent, models), the agent loop diagram, what Shane did
   vs what agents did, failures (System Restore rollback, GPU device loss, backwards guards), the full AI usage log.
   Make the GitHub repo public and link it: full history is the strongest glass-box evidence there is.
2. **OpenAI on screen.** Delegate to Codex / GPT-6 Astra: generate with `gpt-image-2` (Meshy text-to-image, about
   9 credits each, ~760 credits left) the framed museum paintings on the set walls, the four painted backdrops, and
   the itch cover / title key art. Same commit updates the OpenAI award answer and declarations.
3. **Visual pass for Render Royalty** (Claude, 1 block): richer edit-view set (wall/floor materials instead of flat
   blocks, warmer key light), film grain + vignette, a slow camera drift in the edit view. Screenshot-verified.
4. **Rodin prop.** One hero desk prop (e.g. a vintage film projector or director's chair) via Hyper3D Rodin, imported
   with `mj_setup.py`, placed on the desk. Rodin goes in "Tools you used".
5. **Human checks (Shane only):** 20 min playtest with sound (Audio score: nobody has listened yet), a friend plays
   Reel 1 cold for 10 min (Fun / Level design), final video with a monitor on.

## B. Decide
1. **DLSS (NVIDIA award).** Needs NVIDIA's DLSS 4.5 plugin for UE 5.8, downloaded by Shane. Agent wires it in as a
   Graphics option shown only on RTX hardware, TSR elsewhere; package and check the fallback on the Radeon PC.
   Timebox 45 min; revert if packaging breaks. Honest disclosure: "integrated, untested on RTX hardware" unless
   someone with an RTX card runs the build. Worth it: the prize goes to the best qualifying entry, not the winner.
2. ~~Student status~~: not a student (Shane, 14 Nov).

## Decisions taken (14 Nov, 13:30)
- Meshy credits approved. A2 delegated to Codex / GPT-6 Astra (cap 200 credits): gpt-image-2 backdrop atlases and key art.
- Repo goes public for Glass Box once Shane confirms the commit author email may be visible (history scanned: no keys).
- DLSS: Shane unsure. Project moved to the UE 5.9 MegaJam Preview; NVIDIA's plugin targets released engine versions,
  so check for a 5.9 build first; without one, skip.

## Schedule (Claude: about 1 h per 5 h window)
- Sat 13:00: this plan; delegate A2 (Codex) and A4 (Codex, has Rodin OAuth) in parallel.
- Sat 18:00: A3 visual pass; import A2/A4 output.
- Sat 23:00: B1 if Shane has the plugin; package.
- Sun 09:00 content freeze: A1 final post drafted; package `-Clean`.
- Sun 13:00–17:00: video, itch page, devlogs published, submit. Buffer to 01:59.
