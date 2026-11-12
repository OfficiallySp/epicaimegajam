# Game-jam tool health — 12 November 2026

Checked on the Windows jam PC at approximately **10:30–10:38 GMT**, using Codex / GPT-6 Astra in T3 Code.
These are connection and functional smoke tests, not exhaustive tests of every tool operation.
No paid generation, Claude inference, remote publication, or game-content changes were performed.

## Project and global MCPs

| Integration | Result | Evidence and limits |
|---|---|---|
| Unreal MCP | **PASS** | Editor was initially closed. Started the existing UE 5.8.4 project; initialization, discovery of **52 toolsets**, actor selection, PIE state, ten-second PIE start/stop, and Output Log reads passed. PIE interval: **0 Error/Fatal lines and 0 warnings**. Editor left running, PIE stopped. |
| Meshy | **PASS** | Local stdio server exposes **28 tools**. Authenticated `meshy_check_balance` passed. No generation or export tested. |
| fal | **PASS — connection** | `claude mcp get fal` reports **Connected** using saved OAuth. Unauthenticated endpoint returns the expected HTTP 401. Credit entitlement and generation remain untested. |
| Hyper3D / Rodin | **PASS — connection** | `claude mcp get hyper3d-rodin` reports **Connected** using saved OAuth. Unauthenticated endpoint returns HTTP 401. Workspace credits and generation remain untested. |
| Claude Docs | **PASS — connection** | Claude's registered connector reports **Connected**. No model inference used. |
| Melty | **PASS — connection; token rotation needed** | Global MCP reports **Connected**. Its detailed CLI status unexpectedly printed its saved bearer token in the diagnostic tool output. The token is intentionally omitted from this report; rotate it in Melty and update the local connection before further use. |
| Unity MCP | **PARTIAL / unavailable tools** | Repeatedly reports **Connected · tools fetch failed — Request timed out**. No Unity editor was running. MCP transport alone is not proof that editor tools work. Not a blocker for this Unreal project. |

The four project MCP definitions are in `.mcp.json`; Claude also has local-scope definitions.
Claude warns that Meshy's relative project command and absolute local command differ. Both resolve to the
same installed launcher in this checkout, and the connection works. No configuration was removed.
These connections were tested through Claude's model-free CLI and the repository's MCP SDK scripts;
they are not thereby added to the native tool list of the current Codex session.

Initial Meshy/fal/Hyper3D network checks failed inside the restricted shell. Repeating the same read-only
checks outside the network sandbox succeeded. Likewise, `gh auth status` failed in the restricted shell
but succeeded with ordinary keyring/network access. Do not interpret those sandbox failures as expired logins.

## External services and app tools

| Integration | Result | Evidence and limits |
|---|---|---|
| PBRgen | **PARTIAL** | T3 browser opened `https://app.pbrgen.com/`; application and bundled 3D resources loaded. This browser profile showed **Login**, so account credits and export could not be verified. This does not establish the login state in a different browser. Diagnostic tab closed. |
| GitHub CLI / Git remote | **PASS — read access** | Authenticated `gh api user` succeeded; `git ls-remote --exit-code origin HEAD` returned the repository's HEAD. No push performed. |
| GitHub app connector | **FAIL — reauthentication required** | Its user-login tool returned `UNAUTHORIZED` and requested reconnection. GitHub CLI remains usable independently. |
| itch.io / butler | **PARTIAL** | Version and local ZIP audit passed. No credential at butler's configured default identity path. `ITCH_TARGET` and `JAM_TEAM_NAME` are empty in `tools/jam.env`. Upload authentication and publication remain untested. |
| Claude Code | **PASS — CLI/login** | Version **2.1.322**; auth status reports logged in via claude.ai with a Max subscription. Quota reset and model inference were not tested. |
| T3 Code | **PASS — control plane/browser** | Provider discovery, linked-machine discovery, browser opening and page snapshot worked. Mac mini and MacBook Neo links reported reachable; no remote jobs or agent runs started. |
| Plugin management | **PASS — discovery** | Plugin search returned installed GitHub and an available, uninstalled fal plugin. No installations or permissions changed. |
| Sites | **PASS — listing** | Site listing succeeded and returned no owned sites in this account. No site created or deployed. |
| Document control | **PASS — discovery; no document session** | Session discovery succeeded with an empty list. Document operations not tested. |
| Search / web / clock | **PASS — basic calls** | Search connector returned Epic documentation results; built-in web time and clock calls succeeded. |
| Device tooling | **NOT CONFIGURED locally** | Device discovery succeeded but found no devices. Windows cannot host iOS Simulators; Android Emulator and the device helpers are not installed. Not needed for Win64. |

Search probe result: [Epic's Unreal MCP documentation](https://dev.epicgames.com/documentation/unreal-engine/unreal-mcp-in-unreal-editor?application_version=5.8).

## Local development and asset tools

| Tool | Version / setup | Test |
|---|---|---|
| Unreal Engine | **5.8.4** | Existing project loaded; MCP-driven PIE passed. Full rebuild, cooking and packaging not run. |
| Visual Studio / MSVC | VS Community 2026; **14.50.35717** | Compiled, linked and ran a small C++ executable using the selected 14.50 toolset. Windows SDK **10.0.26100.0** is installed. |
| Blender | **5.2.2 LTS** | Factory-startup background session exported a cube to GLB and reimported it; mesh count and dimensions verified. Executable is installed but not on this shell's PATH. |
| ffmpeg / ffprobe | **9.0.1** | Generated a one-second diagnostic tone; verified PCM signed 16-bit, **48 kHz**, mono, duration **1.000000 s**. |
| 7-Zip | **26.05** | Created and tested a ZIP containing the diagnostic WAV; both operations passed. Installed outside PATH; invoked by absolute path. |
| butler | **15.32.0** | Audited that ZIP successfully; no login or upload performed. |
| Node / npm | **24.20.0 / 11.19.0** | Version checks; Node actually ran the MCP clients and Meshy server. |
| Python | **3.12.10** | Standard-library JSON/file write/read round trip passed. |
| Git / Git LFS | **2.55.0.windows.5 / 3.7.1** | Version checks, local repository inspection and Git remote read passed. No LFS upload tested. |
| GitHub CLI | **2.103.3** | Authenticated account API read passed. |
| OBS Studio | **33.0.0** | Installed executable/version verified only; recording and audio capture not tested. |

The compiler probe first compiled successfully but its batch script could not locate the resulting executable
by bare filename. Adding the Visual Studio Installer directory to that probe's process PATH and invoking
the executable by explicit path made the complete compile/link/run check pass. No system PATH was changed.

Scratch inputs and outputs are under ignored `.cache/tool-health-2026-11-12/`; they are diagnostic fixtures,
not game assets. Unreal's detailed smoke report is `.cache/unreal-smoke.json`.

Selected Unreal evidence:

```text
IsPIERunning: {"returnValue":false}
StartPIE: {"returnValue":null}
IsPIERunning: {"returnValue":true}
StopPIE: {"returnValue":null}
IsPIERunning: {"returnValue":false}
LogPlayLevel: PlayLevel: No blueprints needed recompiling
LogPlayLevel: PIE: World Init took: (0.003422s)
LogPlayLevel: Display: Shutting down PIE online subsystems
PIE interval: 0 Error/Fatal lines, 0 warnings.
```

## Remaining readiness work

1. Rotate the Melty token printed by its CLI diagnostic and update its saved connection.
2. Reconnect the GitHub app if native connector use is desired; the CLI already works.
3. Sign into PBRgen in the browser used for material work and verify credits there.
4. Run `butler login`, choose the itch project target and team name, then test uploading when a build exists.
5. Verify fal/Hyper3D credits and one generated asset through export/import when actual asset work starts.
6. Test full Unreal packaging and OBS recording before depending on the shipping/video pipeline.
7. If Unity is needed separately, open its intended project and retry the global relay; otherwise it is irrelevant here.

Paid generators, image generation, destructive/publishing operations, other provider inference, and unrelated
Pets/parental-control/helpline tools were not exercised. An advertised tool is not automatically a tested tool.

## Repeat the core checks

Run serially, with the editor open and no competing Unreal MCP client:

```powershell
node tools/mcp/check.mjs --balance
node tools/mcp/unreal-smoke.mjs
claude mcp list
```

Avoid logging unfiltered `claude mcp get` output for header-authenticated services: it can include secrets.
