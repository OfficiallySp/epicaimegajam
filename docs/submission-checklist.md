# Submission checklist: Sunday 15 Nov (deadline Mon 16 Nov 01:59 GMT; finish by 19:00)

## 13:00, final build (content freeze was 09:00)
- [ ] `git tag -f last-good` on the last known-good commit.
- [ ] Project Settings: Description > Project Name = game name (window title); Maps & Modes > Game Default Map = main
      menu map; Packaging > maps list = shipped maps only, Use Pak File, Use IO Store, compressed cooked packages on,
      Include Debug Files off.
- [ ] `tools\jam.env` has the real `JAM_TEAM_NAME` and `JAM_GAME_NAME`.
- [ ] `.\tools\package.ps1 -Clean`
- [ ] `.\tools\zip-check.ps1` passes: `TeamName_GameName.zip`, under 1 GB.
- [ ] Fresh-folder test: copy the zip to `C:\Temp\final\`, extract, run. Play start → one full loop → quit. Check
      fullscreen, mouse capture, Esc/pause, quit button, audio, no debug text, no on-screen errors.
- [ ] Back up the zip: copy to the Mac and to cloud storage.

## 14:00, video (30–60 s, required)
- [ ] OBS: 2–3 min of the packaged build (not PIE), 1080p60, game audio on. Start with the hook.
- [ ] Cut to 30–60 s (DaVinci Resolve, CapCut or ffmpeg). Optional title card: game name, "2026 Epic AI MegaJam".
- [ ] `ffmpeg -i cut.mov -c:v libx264 -preset slow -crf 18 -pix_fmt yuv420p -c:a aac -b:a 192k -movflags +faststart video.mp4`
- [ ] Upload to YouTube (unlisted or public). Open the link in a private window.

## 16:00, itch.io page
- [ ] Title = game name. Kind: Downloadable. Classification: Game. Windows ticked on the upload. Free, no payments.
- [ ] Upload `TeamName_GameName.zip` through the web dashboard (keeps the exact filename). If that fails, `upload.ps1`
      then rename the upload's display name to `TeamName_GameName.zip` in the dashboard. Download once; check the
      filename and that it runs.
- [ ] Cover image (630×500), 3–5 screenshots, the video embedded.
- [ ] Description: controls, goal, team name, member name, **AI tools used**, **items made before the jam**,
      **third-party assets with licenses** (paste from `docs/asset-declarations.md`), dev-log link. Do not post the
      Epic email on the public page; it goes in the submission form.
- [ ] Visibility: Public (or Restricted if the jam accepts it). The jam must be able to open the page.

## 17:00, submit
- [ ] Jam page > Submit your project > select the game. Form: team name, member name + Epic email, game name, video
      link, dev-log link, declarations (pre-jam items, assets, AI tools).
- [ ] OpenAI Awards: use the prepared answer in `docs/itch-page.md`; credit OpenAI GPT-6 Astra / Codex alongside
      Claude Fable 5.2, Claude Opus 5.5 and the T3 Code orchestrator, with concrete set-kit and prop-pipeline examples.
- [ ] NVIDIA RTX Award: the current build has no qualifying integration. Leave feature boxes unchecked unless an
      eligible feature is added and verified in the packaged build; see `docs/itch-page.md` for the DLSS proposal.
- [ ] Final dev-log post (Glass Box): what AI did, what Shane did by hand, what broke, screenshots, the video link.
- [ ] Confirm the entry appears on the jam's entries page. Screenshot it.
- [ ] `git push`, `git tag submitted`.
- [ ] Optional: post in the Epic forums thread https://epic.gm/2026-ue-megajam.

## If the zip is over 1 GB
1. Packaging > maps list: only the shipped maps (or `package.ps1 -Maps L_MainMenu,L_Level01`).
2. Remove unused packs from `Content/External/` (check References first), and Starter Content if present.
3. Texture sizes: Max Texture Size 1024 on big environment textures; LOD bias via texture groups.
4. Audio: 48 kHz WAV sources are fine; make sure music is not 96 kHz stereo 24-bit.
5. `-nodebuginfo` is already set; make sure no `.pdb` is in the zip (`zip-check.ps1` warns).
6. Re-package `-Clean`, re-check.
