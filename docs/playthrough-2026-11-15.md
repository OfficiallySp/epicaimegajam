# Fix It In Post: complete player playthrough

Played 15 November 2026, approximately 11:57–12:14 GMT, using the packaged Windows game, a fresh save, real mouse/keyboard input, and screenshots between actions. Completed the campaign and credits at 39/41, then replayed the two missed bonus objectives to reach **41/41**. Quit and relaunched: the title screen still showed **41/41**. Your original save was backed up and restored afterward; the completed playtest save is retained separately.

No game source or assets were changed. No console commands, solver, developer answers, scene skips, or save edits were used to progress. Solutions came from the script, board, strips, and studio notes. Source was consulted after finishing the campaign to confirm how stars accumulate.

## Build and evidence

- Executable: `dist/Win64/MegaJam26/Binaries/Win64/MegaJam26.exe`, built 15 November at 11:53:14.
- SHA-256: `D992EA9026B8F03E345FEE4A9165CB2582082D60F6FA55D90BA749B9422B32C9`.
- Engine reported by the running build: `5.9.0-60351404+++UE5+Release-5.9`.
- Window: 1600×900, default High graphics; no `-MJDev` flag.
- [Campaign completion / credits, 39/41](../.cache/playthrough-2026-11-15/44-credits.png).
- [Every scene completed, 41/41](../.cache/playthrough-2026-11-15/52-all-stars.png).
- [41/41 retained after quitting and relaunching](../.cache/playthrough-2026-11-15/54-restart-persistence.png).
- [Campaign log](../.cache/playthrough-2026-11-15/campaign.log), [winning-screening records](../.cache/playthrough-2026-11-15/results.txt), [restart log](../.cache/playthrough-2026-11-15/restart.log).
- [Completed playtest save](../.cache/playthrough-2026-11-15/completed-41-stars.sav).

## Player experience

The campaign is finishable through its normal UI. There was no gameplay crash, soft lock, impossible objective, or progression blocker in this run. Every scene was completed at par during the first campaign pass. The two missed timing bonuses were subsequently earned on separate, over-par attempts; the previously earned par stars remained banked.

The progression works well: cut a conspicuous bad clip, manipulate another character's timing, coordinate characters, then restore missing actions. The script at the top establishes a concrete objective. The miniature sets make the routes readable. Undo/redo and the quick return from failed screenings made experimentation inexpensive. The final scene usefully combines the earlier tools, and keeping the soda advert for its bonus is a good reversal of the opening joke.

The biggest friction was deciding exactly where a dragged clip would land. The most demanding reasoning was the timing between pressure switches and another actor's door crossing. I could solve it from the visible strips, but a clearer explanation of when a switch affects a door would reduce trial and error.

## Findings

### 1. Splice placement is surprising when moving a long clip right — usability, medium

In **Hold The Door**, I dragged the two-beat TEXTING clip right, intending to insert it among PIP's moves. It landed after all four moves, producing a failed screening. Undoing and dropping farther left placed it after the third move and solved the scene.

The effective insertion position follows the strip after the dragged clip has been removed, so a long clip's disappearance changes the relationship between the original visible gap and the final placement. This is recoverable, but easy to misread on the first splice tutorial.

Evidence: [unintended placement and failure](../.cache/playthrough-2026-11-15/17-r2s7-result.png), [corrected placement](../.cache/playthrough-2026-11-15/19-splice-placement.png), [successful result](../.cache/playthrough-2026-11-15/20-r2s7-result.png).

Suggestion: make the insertion boundary and resulting strip order conspicuous while dragging, with a ghost of the clip at its proposed final position.

### 2. Bonus replays make earned stars look lost — usability, medium

After earning the par star in **The Laser Hall**, I replayed it with two cuts to achieve the eight-beat bonus. The result showed the par star dark even though it remained earned overall. The same happened in **The Vault**. Scene selection correctly showed all three stars afterward, and the final total reached 41/41.

This is **not save loss**. The result screen describes this screening, while the scene menu describes cumulative progress. The distinction is not explained on the result screen. It also matters because these two timing bonuses can be pursued separately from the par objective.

Evidence: [Laser Hall bonus result](../.cache/playthrough-2026-11-15/47-laser-bonus.png), [Vault bonus result](../.cache/playthrough-2026-11-15/51-vault-bonus.png), [banked stars](../.cache/playthrough-2026-11-15/52-all-stars.png).

Suggestion: retain a distinct appearance for previously earned stars, and add “Stars are kept across screenings.”

### 3. Packaged material warnings — confirmed log findings, medium technical priority

The successful campaign log contains:

- 42 warnings that `SM_sight_tile` uses a translucent material on a Nanite mesh.
- 41 equivalent warnings for `SM_alert_mark`.
- Missing Nanite usage flags on the ACE, PIP, and guard materials; the log explicitly says the default material will be used in game.

The sight indicators and characters were readable in the tested configuration, so I cannot claim these warnings caused a visible gameplay failure. They are still asset/material setup defects worth checking in the packaged build, especially on another renderer or GPU. The successful campaign log contained **zero `Error:` entries**.

Suggestion: configure the translucent helper meshes to render without Nanite, and validate/cook the figurine materials with the usage flags their meshes require.

### 4. Credits name the wrong engine version — confirmed text defect, low

The credits say Unreal Engine 5.8; the running build reports 5.9. See the [credits](../.cache/playthrough-2026-11-15/44-credits.png) and campaign log header. Update the credit or omit the minor version.

### 5. One transient missing word after loading a scene — low confidence, low

Immediately after returning to **The Vault** through scene selection, one captured frame omitted “ACE” at the start of the script. Later frames displayed it correctly. This was not a persistent obstruction, and I did not establish a reliable reproduction. The log also contains font-atlas flush messages, but that does not establish the cause.

Evidence: [missing first word](../.cache/playthrough-2026-11-15/50-vault-replay.png), [subsequent correct text](../.cache/playthrough-2026-11-15/51-vault-bonus.png).

### 6. Dense strips need careful reading — subjective polish

Small padlocks, short labels, and several aligned strips become harder to scan in scenes 12, 16, and 18. The information is present and the objectives were solvable, but larger lock indicators or stronger hover emphasis would help. This was assessed only at 1600×900.

## Scene-by-scene record — solution spoilers

| Scene | First successful edit | Edits / par | Beats | Final stars |
|---|---|---:|---:|---:|
| 1. Product Placement | Cut SODA AD | 1 / 1 | 6 | 2/2 |
| 2. The Laser Hall | Cut SELFIE; later also cut COMB HAIR for the bonus | 1 / 1; bonus 2 | 9; bonus 8 | 3/3 |
| 3. The Night Watchman | Cut the watchman's opening STARE | 1 / 1 | 6 | 2/2 |
| 4. The Scenic Route | Cut DIVE and LEAP | 2 / 2 | 5 | 2/2 |
| 5. The Diamond | Cut TIE SHOE, keep ADMIRE | 1 / 1 | 12 | 3/3 |
| 6. The Vault | Cut SELFIE and guard GLARE; later also cut WINK | 2 / 2; bonus 3 | 12; bonus 11 | 3/3 |
| 7. Hold The Door | Splice TEXTING after PIP's third move | 1 / 1 | 7 | 2/2 |
| 8. Pull | Move PULL after ACE's second move | 1 / 1 | 7 | 2/2 |
| 9. Sucker Punch | Move WARM UP after the approach; move BREATHE to the start | 2 / 2 | 9 | 2/2 |
| 10. Dead Weight | Cut PEP TALK | 1 / 1 | 12 | 3/3 |
| 11. Look Both Ways | Cut WAIT FOR CAR | 1 / 1 | 12 | 2/2 |
| 12. Two Man Job | Move HUMS onto ACE's switch; move PIP's WAVE to the start | 2 / 2 | 11 | 2/2 |
| 13. The Deleted Scene | Restore GRAB at the diamond | 1 / 1 | 7 | 2/2 |
| 14. The Monologue | Restore MONOLOGUE before SPRING TRAP | 1 / 1 | 8 | 2/2 |
| 15. Hit And Run | Restore FLOOR IT after the car's two existing moves | 1 / 1 | 10 | 2/2 |
| 16. PIP's Big Scene | Cut HESITATES; restore PUNCH after PIP moves up | 2 / 2 | 14 | 2/2 |
| 17. Smile For The Camera | Restore GLITCH at the start of the camera strip | 1 / 1 | 10 | 2/2 |
| 18. The Showdown | Cut PIP's fourth move; restore MONOLOGUE before the trap and GRAB at the diamond; keep SODA AD | 3 / 3 | 14 | 3/3 |

Two failed edited screenings occurred during the campaign: I cut the wrong move in scene 4, and made the misplaced splice in scene 7. Both recovered through the normal controls. These were not progression bugs. A later theatrical rewatch failed as intended and returned to the edited cut without losing its cuts.

## Coverage and limits

Verified: fresh-start onboarding; all 18 scenes; cut, splice, and restore; undo/redo; failed-screening recovery; theatrical rewatch preserving edits; pause/menu navigation; scene selection and bonus replay; credits; all 41 stars; normal quit; persistence after relaunch.

Not assessed: audio quality or mix by listening, controller input, every hint, every alternate solution, fullscreen transitions, other resolutions, hardware compatibility, or measured frame pacing. This was an interaction-and-screenshot playthrough, not a continuous audiovisual recording.

Before gameplay, sandboxed launches stalled with repeated D3D11 GPU-query timeouts and desktop-capture failures. Running the game and input helper outside that sandbox resolved the launch/display problem. The PC was unlocked with its screens on. This is recorded as a test-environment issue, not a demonstrated normal-player startup bug. The failed-launch log is retained as [launch-stall.log](../.cache/playthrough-2026-11-15/launch-stall.log).
