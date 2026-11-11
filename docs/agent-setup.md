# Claude and MCP setup — 11 November 2026

Shane's selected services are **Meshy, fal, Hyper3D and PBRgen**. Use Unreal + Claude Code for development,
Meshy/Hyper3D for 3D assets, fal for images/audio, and PBRgen's web/export workflow for materials.
Keep Git/LFS and Blender. Account setup does not by itself confirm perk redemption or API entitlement.
The account's Claude allowance resets on 12 November; this setup used Codex and model-free CLI/MCP checks.
No Claude inference, paid generation, game assets, or perk redemptions were performed.

## Installed and configured

| Component | Verified state on this PC |
|---|---|
| Claude Code | 2.1.322; existing default remains `opus` |
| Build tools | Visual Studio Community 2026; MSVC **14.50.35717** and 14.51 installed |
| Supporting tools | Node 24.20.0, npm 11.19.0, Python 3.12, Git, Blender 5.2, 7-Zip, ffmpeg, butler |
| Unreal | **5.8.4**, found at `C:\Users\Shane\Epic Games\UE_5.8`; minimal C++ project compiled and opened successfully |
| `unreal-mcp` | **Connected** in Claude at `http://127.0.0.1:8000/mcp`; loopback listener confirmed in editor log; **52 toolsets** discoverable |
| `meshy` | Official server **0.6.11** installed locally; Claude reports **Connected**; direct MCP handshake lists **28 tools** |
| Meshy account | Key supplied securely and saved in ignored `.claude/settings.local.json`; read-only online balance check **passed** on follow-up |
| `fal` | Registered at `https://mcp.fal.ai/mcp-relay`; browser authorization succeeded; Claude reports **Connected** |
| `hyper3d-rodin` | Registered at `https://api.hyper3d.com/api/mcp`; browser authorization succeeded; Claude reports **Connected** |
| PBRgen | Account set up by Shane; web/export workflow, no verified public MCP |

The shareable configuration is `.mcp.json`. The same four servers were registered at local scope
in this PC's Claude configuration, so they are usable here without auto-approving all project
servers. Claude uses the local definitions in preference to matching project entries. Existing global MCPs
were preserved. If the repo moves, update the local Meshy path or remove the four local entries and approve
the project entries in Claude. This setup targets **Claude Code**, not the Claude Desktop Chat tab.

Meshy's dependency versions are locked in `tools/mcp/package-lock.json`; its launcher uses `node` directly,
avoiding Windows `npx.ps1` execution-policy issues and first-use downloads. On a fresh clone:

```powershell
npm.cmd ci --prefix tools/mcp --ignore-scripts
```

The key is referenced by environment variable, never stored in `.mcp.json`. Local settings and dependency
folders are ignored by Git. Do not paste local settings into logs or commit them.
[Claude configuration and scope rules](https://code.claude.com/docs/en/mcp),
[local environment settings](https://code.claude.com/docs/en/settings-reference#env),
[official Meshy server](https://github.com/meshy-dev/meshy-mcp-server).

## Open the editor and reconnect services

1. **Open Unreal** with `.\tools\open-editor.ps1` from this repo. The existing per-user installation was
   found by reading the install registration outside the restricted process; the earlier missing-engine
   finding was incomplete. `MegaJam26.uproject` now enables **Unreal MCP**, **All Toolsets**, **Terminal**,
   **Python Editor Script Plugin** and **Editor Scripting Utilities** for editor targets only. MCP auto-start
   is saved in `Config/DefaultEditorPerProjectUserSettings.ini`. Open the project before connecting Claude.
   This is a **minimal C++ shell**, not a Third Person template project: optional templates were not installed.
   No custom game content has been created. Compilation and a ten-second MCP-driven PIE test passed.
2. **fal is authenticated and connected.** If reconnection is needed later, run the following in an ordinary
   interactive PowerShell terminal from this repo. It does not need a Claude model or subscription quota:

   ```powershell
   claude mcp login fal
   claude mcp get fal
   ```

   Choose the **personal** fal account holding the jam credits in fal's **MCP & connectors** dashboard.
   Website account selection and Active MCP account selection are separate.

3. **Hyper3D is authenticated and connected.** Reconnect if needed with `claude mcp login hyper3d-rodin`,
   selecting the workspace holding the jam credits. Check with `claude mcp get hyper3d-rodin`. The official hosted MCP uses workspace authorization,
   so no Hyper3D API key is needed. [Hyper3D setup](https://hyper3d.ai/features/mcp).

After opening Unreal, use `claude mcp get unreal-mcp` and `claude mcp get meshy`. A successful local
Meshy handshake only verifies the server; the separate read-only balance check below also passed on follow-up.

```powershell
node tools/mcp/check.mjs                 # serial handshakes; no model or generation calls
node tools/mcp/check.mjs meshy --balance # optional read-only account validation
node tools/mcp/unreal-smoke.mjs         # runs then stops a ten-second PIE test; no concurrent Unreal MCP client
```

The diagnostic's fal and Hyper3D requests are deliberately unauthenticated: HTTP 401 proves the endpoint
answers, not that Claude's saved OAuth login works. Use `claude mcp get <server-name>` for that.
Initial online checks were blocked by sandbox/network restrictions and Windows process-launch errors.
An elevated follow-up verified Meshy's account access successfully. Both OAuth flows subsequently completed,
and Claude's individual fal and Hyper3D health checks reported **Connected**. No generation credits were spent.

In the editor, `ModelContextProtocol.GenerateClientConfig ClaudeCode` can refresh the Unreal project
entry. If the port changes, also update this PC's higher-priority local `unreal-mcp` entry. Keep one client
and one tool call at a time against Unreal. Start with listing tools and inspecting a selected actor.
[Epic's UE 5.8 MCP setup](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-mcp-in-unreal-editor?application_version=5.8),
[fal Claude Code setup](https://fal.ai/docs/documentation/setting-up/mcp/claude-code).

Unreal's three visible tools (`list_toolsets`, `describe_toolset`, `call_tool`) are expected: they discover
and dispatch the 52 toolsets on demand. Actor selection, PIE state/start/stop and Output Log reads were tested.
The smoke report is saved in ignored `.cache/unreal-smoke.json`. The default scene stays unsaved and contains
only engine template content. The Game Features asset-manager rule in `Config/DefaultGame.ini` is editor-only,
because that plugin is currently loaded for tooling rather than game features.
The unused Android File Server plugin is disabled for this Win64 setup; its generated authentication
settings are excluded from the shared project configuration.

Claude Code 2.1.322's health check sends an optional `server/discover` request that UE 5.8.4 logs as unknown
before completing standard MCP initialization. It still reports **Connected**, and tool calls pass. This
client-probe log line is separate from game/PIE errors. The editor startup was clean after adding the missing
GameFeatureData asset-manager entry. Full game packaging and Third Person template import remain untested.

## Fable 5.2 decision

**Replace Fable 5.1 for hard tasks; keep Opus 5.5 as the everyday default.** This is a jam-budget/workflow
recommendation, not a measured comparison on this game. The repo's current runbook now reflects it.
The installed Claude Code supports 5.2 (minimum 2.1.321); no upgrade is needed.

After the subscription resets, select `/model claude-fable-5-2`, then `/effort medium`. Raise to `high`
for difficult C++/packaging investigations; reserve `xhigh` for stubborn blockers. Explicit model ID avoids
gateway alias differences. Check `/usage`; leave paid overage and fast mode off. The jam's API-credit offer
is separate from the Claude subscription and does not reset its allowance.
[Fable 5.2 release](https://www.anthropic.com/claude-fable-and-mythos-5-2),
[Claude model/version/effort configuration](https://code.claude.com/docs/en/model-config).

## Which partner tools to use

- **Meshy:** primary 3D generation. Jam offer says web/API credits are usable; validate the actual balance.
- **fal:** images, textures, and optional audio. Authenticate through OAuth; use the correct credited account.
- **Hyper3D/Rodin:** selected alternate for 3D assets. Official MCP is
  `https://api.hyper3d.com/api/mcp`, registered with browser sign-in and workspace selection.
- **Tripo:** use the web perk first. Studio credits and API/CLI/MCP credits are separate; installing its
  official `tripo mcp` would not make the Studio perk spendable through Claude.
- **PBRgen / WeLoveIndies:** manual material/audio downloads. Keep the audio license; the stated perk is
  non-commercial. No public operational MCP verified for either.
- **Rokoko / iClone:** defer until a specific animation need justifies the setup. Rokoko's HTTP Command API
  is not MCP; the verified iClone MCP bridge is community-maintained.
- **Diversion:** retain the existing Git workflow. No reason to migrate version control immediately before kickoff.
- **Cursor/Grok / NetApp+AWS:** optional alternate agents and a post-jam build-pipeline consultation;
  neither is a required asset/editor MCP in this stack.
- **NVIDIA prize:** do not make it the default target on this AMD-only PC; dedicated NVIDIA-feature testing
  would be extra work beyond the basic entry.

Full server commands, billing distinctions, and primary sources are in `docs/partner-mcp-research.md`.
Record tools actually used and asset licenses in `docs/asset-declarations.md`; a configured tool is not yet a used generator.

## Dates and claims (London / GMT)

From the invitation provided by Shane; private codes and redemption links are intentionally omitted.

- **Wed 11 Nov, 19:00:** theme reveal. Prep is allowed; no game-specific work before the jam opens.
- **Thu 12 Nov, 05:00:** jam opens.
- **Thu 12 Nov, 08:00:** Anthropic credit claim opens — three hours after jam opening. Use the original
  email's link. The supplied message did not include its actual target URL.
- **Sun 15 Nov, 19:00:** our planned submission; **Mon 16 Nov, 01:59:** hard deadline.
- PBRgen availability ends Nov 15. Cursor/Grok credits must be claimed by Nov 15 and last seven days.
- Hyper3D claim by Nov 16; benefits expire Nov 20. Tripo access expires **Nov 18, 08:00 GMT**.
- Meshy: new-account offer by Nov 17; card required and auto-renews after the free month.
- Diversion claim by Nov 22; Anthropic by Nov 28, 07:59 GMT or earlier if exhausted; fal by Nov 30.
- Rokoko sends its voucher when the jam starts. iClone Personal is for first-time users, with up to 30 exports.

No offer was redeemed and no subscription or renewal was activated by this setup.
