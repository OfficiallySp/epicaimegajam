# itch.io page copy: Fix It In Post

Paste into the itch.io project page. Items in [brackets] are for Shane to fill in. Screenshots: `devlog/img/`
(03 title, 08 backdrops, 05 reel 3, 04 reel 2, 09 packaged). Video: `dist/video/FixItInPost_gameplay.mp4`
(re-record any time with `tools\make-video.ps1`, then upload to YouTube).

---

## Title
Fix It In Post

## Tagline (short description)
The studio butchered your spy movie. Sneak into the cutting room and recut it. A puzzle game played entirely on an
editing timeline.

## Description

**Every level is a finished movie scene that goes wrong.**

On the editor's desk stands a miniature film set. The actors are toy figurines, and each one has a strip of film: one
clip per action. All the strips run in sync. Watch the studio's theatrical cut fall apart, then fix it the only way a
director can: **in post**.

- **CUT** a clip and everything after it happens one beat earlier. Cut the soda advert and your star is past the guard
  before the guard wakes up.
- **SPLICE** a clip somewhere else on its strip. The sidekick was always going to stand on the switch; now he does it
  while it matters.
- **RESTORE** a deleted scene. The villain's lines are contractually locked, but the studio cut his monologue. Put it
  back and he is still talking when the hero walks out.

The scene re-simulates the instant you touch it, and you can scrub the playhead back and forth like in an editing
suite. When you think it works: roll film.

**18 scenes in 3 reels**, each with an edit budget, a par, and a studio note for a third star (some need a completely
different cut). A hint button names one edit if you are stuck.

Theme: **Director's Cut**. Your vision, your edits, your final cut. The shelved clip that never made it is a real
mechanic here: the Deleted Scenes bin.

## How to play
- Read the script line at the top: that is how the scene should end.
- Click a clip and press **X** (or drag it off its strip) to cut it. Drag a clip along its strip to splice it. Drag a
  clip from **Deleted Scenes** onto its strip to restore it.
- Drag the ruler to scrub through the scene. Coloured tiles show where guards are looking.
- **Space** rolls film. **Z / Y** undo and redo. **R** resets to the theatrical cut. **H** asks for a hint.
  **F** (hold) fast-forwards a screening. **Esc** opens the menu. **F11** toggles fullscreen.

Mouse and keyboard. Windows 64-bit. Unzip and run `MegaJam26.exe`.

## Credits
- Directed by Shane (team officiallysp, solo entry)
- Design, code, level design and edit: Claude (Anthropic's Fable 5.2 model) working as the developer in Claude Code,
  driving Unreal Engine 5.8 through Epic's Unreal MCP. No line of game code was written by a human.
- Dev log: [link]. It is written by the agent, in the first person, mistakes included.

## How AI was used (jam disclosure)
- **Claude Fable 5.2** (Claude Code): concept, game design, all C++ (simulation, solver, set builder, custom Slate UI),
  level design verified by a brute-force solver, editor automation, capture and packaging scripts, dev log.
- **Sub-agents briefed by Claude**: Claude Sonnet 5.5 ran audio generation; Codex / GPT-6 Astra ran prop generation
  and wrote the Blender script for the set kit.
- **Meshy**: the four figurines and nine props (concept image, then image-to-3D).
- **fal** (sonilo sound effects, ElevenLabs music): all 31 sound effects and 3 music loops.
- **Blender** (scripted): the 20-piece modular set kit. **Pillow** (scripted): the painted backdrops.
- Full table, prompts and manifests: `docs/asset-declarations.md`, `assets/src/*/` in the repository.

## Made before the jam opened
Tooling only: agent instructions, packaging / zip / upload scripts, checklists, MCP configuration and an empty C++
project shell. No game code, design or assets. Details in `docs/asset-declarations.md`, section A.

## Third-party content
Unreal Engine 5.8 (Epic Games) and its default Roboto / Droid fonts. Fonts: Bebas Neue and Courier Prime (SIL Open Font
License 1.1, via Google Fonts). No Fab or marketplace assets.

---

## Submission form (not for the public page)
- Team name: officiallysp (solo)    Member: Shane, Epic account email: [ ]
- Game name: Fix It In Post
- Gameplay video (30 to 60 s): [YouTube link]
- Dev log: [itch devlog or repository link to `devlog/`]
- Zip: `dist/officiallysp_FixItInPost.zip` from `tools\zip-check.ps1` (names come from `tools/jam.env`)
