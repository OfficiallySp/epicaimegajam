# tools/ — package, zip, upload

| Script | Runs on | What it does |
|---|---|---|
| `package.ps1` | PC | UAT `BuildCookRun` → `dist/Win64/Windows/` (Development, pak + IO store, compressed, no PDBs, prerequisites installer). Skips the editor rebuild when the editor is open. |
| `zip-check.ps1` / `zip-check.sh` | PC / Mac | Zips to `dist/TeamName_GameName.zip` (exe at the root, `*.pdb` excluded) and fails if the name or the 1 GB limit is wrong. `-CheckOnly` / `--check-only` for a zip downloaded from itch. |
| `upload.ps1` / `upload.sh` | PC / Mac | `butler push` to `ITCH_TARGET:ITCH_CHANNEL` with a date-time-sha user version, then `butler status`. |
| `package.sh` | Mac | Fallback macOS package, only if the PC is unavailable. |
| `common.ps1` / `common.sh` | both | Shared helpers; read `jam.env`. |
| `jam.env` | both | Team and game names, itch target and channel, engine roots. Environment variables with the same names override it. |

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
