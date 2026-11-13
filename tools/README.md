# tools/ — package, zip, upload

| Script | Runs on | What it does |
|---|---|---|
| `package.ps1` | PC | UAT `BuildCookRun` → `dist/Win64/Windows/` (Development, pak + IO store, compressed, no PDBs, prerequisites installer). Skips the editor rebuild when the editor is open. |
| `zip-check.ps1` / `zip-check.sh` | PC / Mac | Zips to `dist/TeamName_GameName.zip` (exe at the root, `*.pdb` excluded) and fails if the name or the 1 GB limit is wrong. `-CheckOnly` / `--check-only` for a zip downloaded from itch. |
| `upload.ps1` / `upload.sh` | PC / Mac | `butler push` to `ITCH_TARGET:ITCH_CHANNEL` with a date-time-sha user version, then `butler status`. |
| `package.sh` | Mac | Fallback macOS package, only if the PC is unavailable. |
| `common.ps1` / `common.sh` | both | Shared helpers; read `jam.env`. |
| `jam.env` | both | Team and game names, itch target and channel, engine roots. Environment variables with the same names override it. |
| `mcp/package.json` + lockfile | PC / Mac | Pinned official Meshy MCP server; restore with `npm ci --prefix tools/mcp --ignore-scripts`. |
| `mcp/check.mjs` | PC / Mac | Read-only MCP handshakes; `node tools/mcp/check.mjs meshy --balance` also validates Meshy account access without generation. See `docs/agent-setup.md`. |
| `open-editor.ps1` | PC | Opens this repo's project using `UE_ROOT_WIN` from `jam.env`; MCP auto-starts through project settings. |
| `mcp/unreal-smoke.mjs` | PC | Checks selection and PIE state, runs ten seconds of PIE, stops it, and reads the Output Log. Refuses to interrupt an existing PIE session. Run with no other Unreal MCP client. Report: ignored `.cache/unreal-smoke.json`. |

## One-time setup
- Fill in `tools/jam.env` (team name, game name, `ITCH_TARGET=user/game-slug`).
- `butler login` on each machine that uploads (opens a browser; the token is stored in the itch config folder).
- PC: 7-Zip installed (`winget install 7zip.7zip`) for fast zips; without it the script falls back to `Compress-Archive`.
- PowerShell: run from the repo root as `.\tools\package.ps1`. If scripts are blocked: `Set-ExecutionPolicy -Scope CurrentUser RemoteSigned`.

## Typical day
```powershell
.\tools\package.ps1          # 5–30 min depending on shader/cook state
.\tools\zip-check.ps1        # dist\TeamName_GameName.zip
# extract the zip into a NEW folder, run the exe, play one loop
.\tools\upload.ps1           # butler push to the iteration channel
```

## Final submission
Upload the exact zip through the itch.io web dashboard so its filename stays `TeamName_GameName.zip`
(itch re-serves butler-pushed builds under its own filename). Keep `upload.ps1` for iteration builds and as a
fallback; if the fallback is used, rename the upload's display name in the dashboard and download once to check.
Full list: `docs/submission-checklist.md`.

## Added during the jam (agent tooling)
| Script | What it does |
|---|---|
| `rebuild.ps1` | Closes this project's editor, full editor build (`-NoHotReloadFromIDE`), reopens it and waits for Unreal MCP. |
| `mcp/unreal-call.mjs` | Calls any Unreal MCP toolset tool from the shell: `py "@script.py"`, `console "mj.Dump"`, `pie start|stop`. |
| `mcp/meshy-call.mjs` | Calls the Meshy MCP server from the shell with the key from `.claude/settings.local.json`. |
| `latest-shot.ps1` | Copies the newest `shot showui` screenshot to `.cache/shot.png`. |
| `capture-window.ps1` | PNG of a window's client area (packaged game). |
| `drive-game.ps1` | Plays the packaged build with real mouse and keyboard input: `click`, `drag`, `key`, `shot`, `record`, `expect <log regex>`. Coordinates are the HUD's 1920x1080 canvas. |
| `make-video.ps1` | Records the 58 s gameplay video hands-free: four scenes, window capture plus the in-game F9 audio capture, cut together. Output `dist/video/FixItInPost_gameplay.mp4`. |
| `make_backdrops.py` | Paints the three backdrop atlases with Pillow. |
| `blender/make_kit.py`, `check_kit.py` | Generate and verify the 20-piece set kit. |
