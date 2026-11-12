# Design: FIX IT IN POST

Concept locked Thu 12 Nov 2026, 14:30 GMT (after the 05:00 jam start). Chosen and written by Claude Fable 5.2 under
Shane's brief: "it is up to you how you plan this out". Theme: **DIRECTOR'S CUT**.

## 0. Ideation (Thu 12 Nov, after jam start)
| # | Concept (one line) | Theme link | Core verb | Why it could be fun in 30 s | Risk |
|---|---|---|---|---|---|
| 1 | **Fix It In Post**: every level is a finished scene that goes wrong; re-edit each actor's film strip so it goes right | You make the director's cut, literally | cut / splice / restore clips | Cut one clip, watch the whole scene change | Puzzle readability; UI heavy |
| 2 | Take Two: play a scene several times as different roles, earlier takes replay beside you | Takes, "cut!", final montage | run / shoot | Self co-op is proven fun | Known genre; needs feel tuning an agent cannot playtest |
| 3 | Reverse AI director: you spawn set pieces to keep an AI action hero alive and the audience excited | You direct the AI | place / trigger | Pacing a scene live | Tuning heavy, weak progression |
| 4 | Film-strip platformer: cut frames out of the level so it contracts | Cutting the film | jump / cut | Spatial "aha" | Platformer feel + level art cost |
| 5 | Blade-wielding director slicing a set apart | Pun only | slash | Juice | Thin theme, crowded idea |

Finalists: 1 and 2. **Chosen: 1.** It is the most literal and least crowded reading of the theme, its fun lives in
deterministic logic (which an AI agent can verify with a solver instead of by feel), and the result of every edit is
a little movie, which gives the visuals and audio a clear job.

## 1. Pitch
The studio butchered your spy movie. Sneak into the cutting room and fix it in post: every scene is a miniature
set where the actors replay their footage, and you cut, splice and restore clips on their film strips until the
scene ends the way you shot it.

## 2. Theme
- The player's only verbs are an editor's: **cut**, **splice** (reorder), **restore deleted scenes**. The win state
  of every level is literally "the director's cut" of that scene; the starting state is "the theatrical cut".
- The announcement's "feature that didn't quite make it" is the *Deleted Scenes* bin: shelved clips you put back.
- Studio interference is the antagonist: product placement, focus-group notes, locked ("contractual") clips.
- "Directing AI agents": the actors are simple agents that follow their strips; you never control them directly.

## 3. Core mechanic
A scene is a grid set with actors. Each actor has a **track**: a strip of **clips**, one action each (move, wait,
look, use, attack, or a multi-beat take). All tracks run in sync, one beat at a time. Cutting a clip ripples
everything after it one beat earlier, so a single cut re-times that actor against everyone else. The scene is a
deterministic simulation, re-run instantly after every edit; the playhead can be scrubbed like in an editing suite.
- Edits are limited per scene (the "edit budget"). The count is a diff against the theatrical cut, so undoing or
  re-doing never wastes budget: deleted clips + restored clips + minimum number of clips moved.
- Some tracks or clips are locked by the studio.

Simulation rules (per beat): 1) all moves resolve simultaneously (walls, closed doors and standing actors block;
head-on swaps bump); 2) actions resolve on the new positions (use = pull lever / grab item on your tile; attack =
knock out whoever is on the tile in front); 3) the world reacts (plates, levers, doors, lasers); 4) fail checks
(star dead, star seen by a guard's line of sight); 5) win check.

## 4. Loop (30 seconds)
Watch the theatrical cut fail (the star walks into a laser because the studio added a soda advert) → read the
script line for how the scene should end → cut / drag / restore clips, scrubbing to see the consequences →
**ROLL** → the scene plays as a film; "CUT!" with the reason if it fails, "PRINT IT!" and a star rating if it works.

## 5. Win and lose
- Win a scene: the script's ending happens during a screening (e.g. the star reaches the mark with the diamond).
- Fail a screening: star knocked out or spotted, or the film runs out. No penalty beyond re-editing.
- Stars per scene: ★ printed, ★ under the edit par, ★ scene-specific studio note (bonus objective).
- Win the game: all scenes of all three reels printed; the credits roll over a montage of your cut.

## 6. Progression: three reels
| Reel | What changes | New pressure or tool | Target length |
|---|---|---|---|
| 1. The Heist | Scenes with one editable strip; guards, lasers, product placement | **Cut** only | 4 scenes, 3–5 min |
| 2. The Getaway | Two strips that must sync (star + sidekick), levers, plates, knock-outs | **Splice** (drag to reorder) | 4–5 scenes, 6–8 min |
| 3. The Showdown | Villain and props have strips too (bomb, lasers, the car) | **Deleted scenes** (restore), studio-locked clips | 4–5 scenes, 8–10 min |

## 7. Controls (mouse + keyboard)
Left click a clip to select; **X / Delete** or drag off the strip to cut; drag along the strip to splice; drag from
the bin to restore. Click or drag the ruler to scrub. **Space** roll / stop, **Z** undo, **Y** redo, **R** reset to the
theatrical cut, **F** fast-forward (hold), **Esc** pause menu. Everything also has an on-screen button.

## 8. Art direction
Miniature, theatrical, warm. A tabletop previs set on the director's editing desk at night: toy-figurine actors on
round bases that hop from tile to tile, cardboard-and-wood set pieces, a desk lamp key light, strong tilt-shift depth
of field, film grain and letterbox during screenings. Palette: deep navy shadows, tungsten amber key, one accent
colour per reel (teal / magenta / red). UI looks like strips of film on a light table.

## 9. Audio plan
Music: a sneaky jazz/heist loop for editing (low, brushed drums, upright bass), a fuller arrangement during
screenings, a sting for PRINT IT and one for CUT. SFX (every interaction): clip hover tick, select, scissor snip,
splice tape, restore whoosh, clapperboard on ROLL, projector whirr loop, scrub ratchet, figurine hop, bump, lever,
door, laser hum + zap, knock-out, spotted sting, applause, star pings, menu clicks.

## 10. Scope
Definition of done: one mechanic (editing strips), three reels, menus (start, pause, restart, quit, volume,
fullscreen), audio on every interaction, music, save of progress and stars.
Later (cut list, in priority order):
- Montage of the player's own cuts under the credits.
- Skeletal animation for the actors (figurines hop instead).
- Recast: drag a clip to another actor's strip.
- Bomb / vehicle prop strips beyond the minimum needed by reel 3.
- Gamepad cursor.

## 11. Risks and fallbacks
| Risk | Fallback |
|---|---|
| Puzzle rules unreadable | Outcome marker on the ruler, sight lines drawn on the set, hover-scrub, first three scenes teach one rule each |
| Levels unsolvable or trivial | In-engine brute-force solver (`mj.Solve`) reports solution counts per edit budget for every level |
| Agent cannot feel-test UI | Slate automation toolset + console commands for every action + a self-playing demo mode |
| AI art not ready or inconsistent | Every asset has a primitive fallback in code; the game is shippable with boxes |
| Packaging never tested on this PC | Package on day one, before content exists |
