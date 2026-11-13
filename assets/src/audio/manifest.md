# Audio pack manifest — Fix It In Post

Generated 13 Nov 2026 (GMT) with fal. SFX: `sonilo/v1.1/text-to-sound-effects` (requested as WAV, 44.1 kHz stereo, `duration` as shown). Music: `elevenlabs/music/v2.5` (`force_instrumental: true`, `pcm_48000`). Raw downloads are in `raw/` (`<cue>.wav`). Finals are WAV 48 kHz 16-bit PCM: SFX mono, music and loop processed as below.

Processing (ffmpeg): SFX = down-mix to mono, strip leading silence (-45 dB), strip trailing silence (-50 dB), cap length, 5 ms fade-in, fade-out of min(150 ms, 25 % of length), peak normalised to -3.0 dBFS. `projector_loop` = 10 s source, last 2 s equal-power crossfaded into the first 2 s (8.000 s loop). Music = last 2 s of the usable body crossfaded into the first 2 s, the generated outro (final ~3 s) discarded, then two-pass linear `loudnorm` to -18 LUFS, true peak limit -1.5 dBTP.

Peak figures are sample peak (volumedetect). Cost per call = fal rate x requested duration (SFX $0.0018/s, music $0.60/min); fal's pricing API does not expose the actual billed amount, so costs are estimates.

| Cue file | fal endpoint | Prompt (exact) | Requested s | Final duration s | Peak dBFS | Est. cost USD |
|---|---|---|---|---|---|---|
| click.wav | sonilo/v1.1/text-to-sound-effects | single soft tactile button click, dry, tiny, close-mic, on a wooden desk | 1 | 0.051 | -3.0 | 0.0018 |
| select.wav | same | single light plastic tick, picking up a slide, dry and tiny, close-mic | 1 | 0.140 | -3.0 | 0.0018 |
| pickup.wav | same | short celluloid film strip rustle as it is lifted off a desk, quiet, close-mic | 1 | 0.350 | -3.0 | 0.0018 |
| cut.wav | same | one crisp scissors snip cutting film, single cut, dry, close-mic | 1 | 0.136 | -3.0 | 0.0018 |
| splice.wav | same | sticky tape peel followed by a firm press, splicing film strips together, close-mic, dry | 1.5 | 0.550 | -3.0 | 0.0027 |
| restore.wav | same | quick soft paper and film slide-in whoosh, small, close-mic, dry | 1.5 | 0.500 | -3.0 | 0.0027 |
| unrestore.wav | same | quick soft paper and film slide-out whoosh, lower pitched, small, close-mic, dry | 1.5 | 0.405 | -3.0 | 0.0027 |
| undo.wav | same | tiny tape rewind blip, short, quiet, mechanical, dry | 1 | 0.300 | -3.0 | 0.0018 |
| reset.wav | same | tape rewinding fast then stopping with a small clunk, reel-to-reel, close-mic | 2 | 0.950 | -3.0 | 0.0036 |
| denied.wav | same | dull wooden double thunk, two quick muted knocks, not allowed, close-mic | 1 | 0.350 | -3.0 | 0.0018 |
| scrub.wav | same | single mechanical projector ratchet tick, tiny, dry | 0.5 | 0.070 | -3.0 | 0.0009 |
| hint.wav | same | one small desk bell ding, single ring, gentle, close-mic | 2 | 0.714 | -3.0 | 0.0036 |
| star.wav | same | bright glockenspiel chime ping, single note, sparkly, short | 1.5 | 0.700 | -3.0 | 0.0027 |
| roll.wav | same | film clapperboard clap followed by a 16mm projector motor spinning up | 3 | 1.600 | -3.0 | 0.0054 |
| cutcall.wav | same | clapperboard slam plus a comedic record-scratch film jam, that take is ruined | 2.5 | 1.400 | -3.0 | 0.0045 |
| print.wav | same | short triumphant big-band brass sting with a little applause underneath, finale button | 5 | 3.100 | -3.0 | 0.0090 |
| hop.wav | same | a wooden game piece tapped down on a wooden board, single tap, dry, close-mic | 0.8 | 0.175 | -3.0 | 0.0014 |
| bump.wav | same | soft wooden knock with a slight cartoon bounce, close-mic | 1 | 0.300 | -3.0 | 0.0018 |
| punch.wav | same | cartoon punch whack, single hit, comedic, dry | 1 | 0.350 | -3.0 | 0.0018 |
| whiff.wav | same | fast air swish, a missed punch, single whoosh, dry | 1 | 0.300 | -3.0 | 0.0018 |
| down.wav | same | small cartoon body thud, a toy figurine falling over on a wooden table | 1.5 | 0.550 | -3.0 | 0.0027 |
| lever.wav | same | chunky metal lever clunk, single throw, mechanical, close-mic | 1.5 | 0.450 | -3.0 | 0.0027 |
| door.wav | same | heavy stone slab sliding down into the floor, short stone and wood slide | 2 | 0.700 | -3.0 | 0.0036 |
| switch.wav | same | electrical relay click with a brief power-down hum fading out | 1.5 | 0.550 | -3.0 | 0.0027 |
| zap.wav | same | cartoon laser electric zap, single shot, bright, short | 1.5 | 0.550 | -3.0 | 0.0027 |
| grab.wav | same | sparkly gem pick-up shimmer, magical glittering chime, short | 1.5 | 0.700 | -3.0 | 0.0027 |
| bonus.wav | same | golden coin-like shimmer rising in pitch, bonus pickup, short | 1.5 | 0.800 | -3.0 | 0.0027 |
| spotted.wav | same | short alarming brass dun stab, spy movie style, single hit | 2 | 0.800 | -3.0 | 0.0036 |
| crush.wav | same | comedic car hit, tyre chirp then a squashy thump, cartoon | 2 | 0.800 | -3.0 | 0.0036 |
| fall.wav | same | short slide whistle sliding down, cartoon fall | 2 | 0.800 | -3.0 | 0.0036 |
| projector_loop.wav | same | steady 16mm film projector whirr, continuous mechanical hum and soft rattle, constant level, no start or stop | 10 | 8.000 (loop) | -3.0 | 0.018 |
| music_edit.wav | elevenlabs/music/v2.5 | Instrumental sneaky 1960s heist jazz, medium-slow tempo about 100 bpm, key of D minor: brushed drums, walking upright bass, vibraphone melody, muted trumpet stabs, lots of space, cool and playful, background music for thinking, steady groove with no big build or ending | 80 | 74.400 (loop) | -3.2 (-18.0 LUFS) | 0.80 |
| music_screening.wav | same | Instrumental 1960s big-band spy theme, about 100 bpm, key of D minor, same band as a sneaky heist jazz piece but fuller and driving: punchy brass hits, bongos, surf-style twangy electric guitar, walking bass, drums, energetic and playful, steady groove with no big build or ending | 58 | 52.800 (loop) | -6.8 (-18.0 LUFS) | 0.58 |
| music_title.wav | same | Instrumental stylish cinematic main-title spy jazz theme with a strong memorable hook: twangy guitar and brass melody, bold orchestral stabs, upright bass, bongos, 1960s secret agent film opening, glamorous and mischievous | 58 | 50.400 (loop) | -5.7 (-18.0 LUFS) | 0.58 |

Estimated total spend: about 0.10 USD (SFX, 57.8 s generated) + 1.96 USD (music, 196 s generated) = about 2.06 USD, no regenerations.

Notes on the loops: the music loop lengths (74.4, 52.8, 50.4 s) are multiples of one 2.4 s bar at 100 bpm, but the generated tempo and bar alignment were not measured, so the loop join may sit slightly off the beat. Each join is an equal-power 2 s crossfade, so level is continuous; listen to the loop point once in-engine. In `music_edit` the 1 s mean level at the very start is -17.3 dB against -23.8 dB at the very end, which is the sparse jazz rhythm rather than a join discontinuity (per-second levels in the source vary -15 to -24 dB throughout).

## Licence

Checked 13 Nov 2026 with the fal docs search tool and the model schemas.

- fal FAQ, "Can I use generated files for commercial purposes?": "Each model has its own license. Most models on fal are available for commercial use and are marked with a `Commercial use` badge on the model page. Models marked `Research only` are restricted to non-commercial use. Check the model's page for its specific license." (https://fal.ai/docs/documentation/model-apis/faq)
- `sonilo/v1.1/text-to-sound-effects`: the fal catalogue description reads "Generates high-quality, commercial-use-safe sound effects from a text prompt, with full control over type, texture, intensity, and exact duration." Checked 13 Nov 2026 23:30 GMT: the fal model page shows the **`Commercial use`** badge (no separate licence text; fal's Terms of Service apply).
- `elevenlabs/music/v2.5`: the fal catalogue description ("Generate high quality, realistic music with fine controls using Elevenlabs Music v2.5!") says nothing about licensing, and the fal docs search returned no model-specific licence for it. Checked 13 Nov 2026 23:30 GMT: the fal model page (fal.ai/models/elevenlabs/music/v2.5) shows the **`Commercial use`** badge (no separate licence text; fal's Terms of Service apply).
