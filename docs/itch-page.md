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
- Design, code, level design and edit: Anthropic's Claude Fable 5.2 and Claude Opus 5.5 in Claude Code,
  driving Unreal Engine 5.8 through Epic's Unreal MCP. No line of game code was written by a human.
- Set-kit scripting and prop pipeline: OpenAI GPT-6 Astra through Codex, working alongside Claude through the
  T3 Code orchestrator. Meshy generated the prop images and meshes; Blender generated the modular kit from code. The painted set walls and
  key art are OpenAI gpt-image-2.
- Source, full history and AI usage log: https://github.com/OfficiallySp/epicaimegajam
- Dev log: [link]. It is written by the agent, in the first person, mistakes included.

## How AI was used (jam disclosure)
- **Claude Fable 5.2** (Claude Code): concept, game design, initial C++ (simulation, solver, set builder, custom Slate UI),
  level design verified by a brute-force solver, editor automation, capture and packaging scripts, dev log.
- **Claude Opus 5.5** (Claude Code): playtest fixes, code review, clearer game text, hint and input fixes, and
  screening-camera improvements.
- **OpenAI GPT-6 Astra** (Codex): wrote the Blender generator and re-import checks for the 20-piece modular set kit;
  ran the nine-prop Meshy pipeline, checked the exported models, and supported tooling and Unreal MCP setup.
- **T3 Code orchestrator**: coordinated the Codex and Claude Code agents, including delegated Astra tasks alongside
  Claude Fable 5.2 and Opus 5.5. Shane directed the work and playtested the game.
- **Claude Sonnet 5.5** (sub-agent briefed by Claude): ran audio generation and post-processing.
- **Meshy**: the four figurines and nine props (concept image, then image-to-3D).
- **fal** (sonilo sound effects, ElevenLabs music): all 31 sound effects and 3 music loops.
- **OpenAI gpt-image-2** (through Meshy, directed by GPT-6 Astra): the 12 painted wall flats on every set and the key art.
- **Blender** (scripted): the 20-piece modular set kit.
- Full table, prompts and manifests: `docs/asset-declarations.md`, `assets/src/*/` in the repository.

## Made before the jam opened
Tooling only: agent instructions, packaging / zip / upload scripts, checklists, MCP configuration and an empty C++
project shell. No game code, design or assets. Details in `docs/asset-declarations.md`, section A.

## Third-party content
Unreal Engine 5.9 MegaJam Preview (Epic Games) and its default Roboto / Droid fonts. Fonts: Bebas Neue and Courier Prime (SIL Open Font
License 1.1, via Google Fonts). No Fab or marketplace assets.

---

## Submission form (not for the public page)
- Team name: officiallysp (solo)    Member: Shane, Epic account email: [ ]
- Game name: Fix It In Post
- Gameplay video (30 to 60 s): https://youtu.be/ZDWzAcmWjiA
- Dev log: [itch devlog link]; repository with full history: https://github.com/OfficiallySp/epicaimegajam
- Zip: `dist/officiallysp_FixItInPost.zip` from `tools\zip-check.ps1` (names come from `tools/jam.env`)

### OpenAI Awards: copy for the submission field

We used OpenAI GPT-6 Astra through Codex during development, alongside Anthropic's Claude Fable 5.2 and Claude
Opus 5.5, coordinated through the T3 Code orchestrator. Astra wrote the Blender generator and validation scripts
for the 20-piece modular set kit used to build our miniature film sets. It also ran the nine-prop Meshy pipeline,
including image revisions, model generation, downloads and structural checks. Meshy generated those images and
meshes; Astra directed the tools. Astra also directed OpenAI's gpt-image-2 to paint every wall the player looks at: 12 theatre flats (the museum's
portraits and landscapes, the alley's posters and barred windows, the lair's hazard panels and monitors), assembled
by a script it wrote so any flat tiles next to any other, plus the key art. These contributions became the sets,
walls and props in the playable game. Shane
directed the agents and playtested the results. The dev log records the work and its limitations.

Evidence in this repository: `docs/ai-usage-log.md` (13 Nov), `tools/blender/make_kit.py`,
`tools/blender/check_kit.py`, `assets/src/kit/`, `assets/src/props/README.md`, `tools/assemble_backdrops.py` and
`assets/src/backdrops/README.md` (every gpt-image-2 prompt and task id). Link the published dev log in
the form so judges can follow the evidence.

### NVIDIA RTX Award: current status and possible addition

**Not implemented as of 14 Nov. Leave the NVIDIA feature boxes unchecked for the current build.** The project uses
stock UE 5.8, DX12, Lumen and TSR (`r.AntiAliasingMethod=4`). It has no DLSS, ACE or NvRTX integration. The local
editor log identifies the development GPU as AMD Radeon 8060S.

Shane confirmed that no RTX test machine is available. Recommendation for this jam: defer DLSS and keep the tested
renderer. The integration outline below is a future option, not a claim about the submission build.

The practical candidate is **DLSS Super Resolution**, using NVIDIA's official DLSS 4.5 plugin for UE 5.8.
Proposed scope: an optional DLSS Quality setting for supported RTX hardware, with the existing TSR path on other
hardware. The aim is to reduce rendering cost while preserving detail in the miniature sets; performance and image
quality must be measured in this game. DLAA is a possible native-resolution option, but not needed for the initial
integration. An NvRTX engine migration is a much larger change this close to submission.

Before declaring DLSS: integrate the matching plugin, check support at runtime, package Win64, test the TSR fallback
on the Radeon PC, and test actual DLSS operation on an RTX PC. Check moving figurines, thin props, depth of field,
camera cuts and the timeline UI. Record the build, GPU, driver, selected mode and test results. Installing a plugin
alone does not demonstrate a working feature. No NVIDIA submission claim is ready yet.

### Award references (checked 14 Nov 2026)

- [Jam categories and submission requirements](https://itch.io/jam/2026-epic-ai-megajam): OpenAI development use
  counts; an in-game OpenAI API feature is not required. For this Windows entry, consider Best Overall UE: OpenAI
  Award and Render Royalty: OpenAI Award. Pocket Powerhouse requires a mobile entry. The NVIDIA award is attached
  to Render Royalty and lists DLSS as a qualifying feature; stock Lumen alone is not listed. Final qualification
  belongs to the judges.
- [OpenAI GPT-6 Astra](https://developers.openai.com/api/docs/models/gpt-6-astra): official model name and ID
  (`gpt-6-astra`).
- [NVIDIA DLSS](https://developer.nvidia.com/rtx/dlss): UE 5.8 plugin download, Super Resolution and DLAA descriptions.
