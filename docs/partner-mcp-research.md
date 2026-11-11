# Epic AI MegaJam partner MCP research

Checked **2026-11-11** against public vendor documentation and source repositories. Documentation verification only: no installation, authentication, generation, or credit spending. “No verified MCP” means no public operational MCP was established from the sources checked, not proof that none exists privately. Unreal MCP, Claude model choice, and actual setup belong to the parent task.

## Recommended minimum

**Keep the existing Git repo and Blender; add Meshy for 3D, with fal only when broader media generation is needed.** Use the parent’s Unreal/Claude Code setup as the core. This is a workflow recommendation, not a benchmark of output quality.

- Meshy is a practical first choice because its official server supports generation, processing, and downloads, and its account credits can fund API work. Hyper3D is the alternative when its workspace already has usable credits and browser authentication is preferable.
- Do not enable three competing 3D generators initially. Use Tripo’s web app if the available entitlement is Studio-only; its CLI/MCP needs API credits separately.
- Keep Blender for inspection, cleanup, scale, and export. These generation services run remotely; their documented MCP clients do not require local CUDA inference. The Radeon 8060S therefore does not need to run a local generative model. That does not establish Unreal or iClone performance on this exact GPU.
- Use PBRgen manually for a missing material, WeLoveIndies for licensed audio downloads, and Rokoko only when a specific animation is needed. Defer iClone/community bridges and a Diversion migration for this solo beginner jam.
- During later setup, verify tools/authentication with balance, search, or status calls first. Keep generated files locally and record their provenance in the parent’s asset log. No generation was performed for this research.

## Official generation MCPs

| Partner | Exact connection and authentication | Windows considerations | Do web credits fund it? |
| --- | --- | --- | --- |
| **Meshy — official** | Local stdio: `npx -y @meshy-ai/meshy-mcp-server`; authenticate with `MESHY_API_KEY`. API-key creation requires Pro or above. The vendor links the **meshy-dev/meshy-mcp-server** repository; this is not a community lookalike. No hosted MCP URL is published on the setup page. [Vendor setup](https://www.meshy.ai/mcp), [first-party source](https://github.com/meshy-dev/meshy-mcp-server). | Node.js 18+. Use `cmd /c npx` in Windows MCP configuration; quote the CLI separator as `"--"` in PowerShell. Blender is not required. [Windows instructions](https://www.meshy.ai/mcp). | **Yes, shared account balance:** Meshy explicitly says its API-backed integration draws the same credits as the web app. MCP uses the API key’s account and API operation prices. Check `meshy_check_balance`; do not confuse credit sharing with asset-library sharing. [Credit sharing](https://help.meshy.ai/en/articles/16102955-how-to-use-meshy-in-comfyui), [MCP billing](https://help.meshy.ai/en/articles/16102957-meshy-mcp-server-and-ai-coding-agent-setup), [API pricing](https://docs.meshy.ai/en/api/pricing). |
| **fal — official** | Hosted Streamable HTTP: `https://mcp.fal.ai/mcp-relay`. Browser OAuth; no API key/header for the recommended connection. Generates media and discovers models/prices. **Documentation-only** `https://fal.ai/docs/mcp` cannot generate assets. [Current MCP reference](https://fal.ai/docs/documentation/setting-up/mcp). | Remote HTTP needs no local Python/Node server. Use the Claude command below, not the macOS/Linux Bash installer. [Claude Code setup](https://fal.ai/docs/documentation/setting-up/mcp/claude-code). | **Yes.** Monthly plan credits work across fal products, including API; direct API usage does not receive the UI plan discount. MCP bills standard model rates to the **Active MCP account**, personal by default; this selection is separate from the website account switcher. [Credit-plan rules](https://fal.ai/docs/documentation/setting-up/agent-plans-and-retention), [MCP accounts](https://fal.ai/docs/documentation/setting-up/mcp), [MCP pricing](https://fal.ai/mcp). |
| **Tripo — official current CLI MCP** | Local stdio: **`tripo mcp`**, from npm package `tripo-cli`. Browser authorization with `tripo login --region ov` for the international developer account, or `cn` for mainland China. Alternative: `TRIPO_API_KEY` plus matching `TRIPO_REGION`. [Official CLI docs](https://developers.tripo3d.ai/en/docs/cli). | Node.js 20+. Use `npm.cmd`/`tripo.cmd` or Command Prompt to avoid blocked PowerShell `.ps1` shims. Credentials/config live under `%USERPROFILE%\.tripo`. No Blender dependency. [CLI Windows guidance](https://developers.tripo3d.ai/en/docs/cli). | **No Studio credit transfer.** Tripo Studio subscriptions/credits and API credits are separate. Browser CLI login does not merge them. Confirm API balance before choosing this route. [Explicit billing distinction](https://www.tripo3d.ai/help/api-plugins/tripo-studiotripo-api), [API billing](https://developers.tripo3d.ai/en/docs/billing). |
| **Hyper3D / Rodin — official** | Hosted HTTP: `https://api.hyper3d.com/api/mcp`. Browser authorization, with billing workspace selection; no API key or local server. It generates models and returns downloads, rather than controlling Blender. [Official MCP instructions](https://hyper3d.ai/features/mcp). | Native Claude Code HTTP connection works without a Blender add-on or local inference runtime. Vendor supplies Windows configuration guidance. [Setup](https://hyper3d.ai/features/mcp). | **MCP uses the selected workspace’s plan and credits.** Direct REST instead uses an API plan/credits. Do not assume a web Business entitlement also funds direct REST; use the documented workspace-authenticated MCP route. [MCP/API billing comparison](https://hyper3d.ai/features/mcp). |

Tripo’s current CLI docs describe version 0.9.0 and optional `tripo mcp --toolset full`. Its older official [Tripo MCP repository](https://github.com/Tripo-Developer/tripo-mcp) describes an alpha Blender-add-on bridge. It is **not** evidence that current Tripo MCP requires Blender. Prefer the documented CLI route here.

## Other partners: community bridges or no verified MCP

| Partner / status | Connection, authentication, and Windows | Subscription/API implications and practical route |
| --- | --- | --- |
| **PBRgen — no verified public MCP or developer API** | No verified MCP URL/command/auth scheme. Browser material editor at `https://app.pbrgen.com`; optional trial without signup, account for credits. No local server needed. [Official product/FAQ](https://www.pbrgen.com/). | Web plans provide material-generation credits; no verified API entitlement or credit transfer. Export texture maps or GLB for Blender/Unreal. The [official user-guide URL](https://app.pbrgen.com/user-guide) could not be retrieved by the research fetcher, so this finding is limited to accessible public material. |
| **Diversion — community MCP; official CLI/plugin** | Community `mtuska/diversion-vscode` publishes `npx -y @mtuska/diversion-mcp` (Windows: `cmd /c npx -y @mtuska/diversion-mcp`). Requires Node 18+, `dv.exe`, a Diversion workspace, and its running local agent. It obtains tokens through that agent; optional `DIVERSION_CORE_TOKEN` override. Its platform section still calls real Windows `dv` testing unverified despite a broader support claim elsewhere. [Maintainer’s source](https://github.com/mtuska/diversion-vscode). | No generation-credit pool. Official docs specify **Pro+ for REST API access**; do not assume the community wrapper works on every plan. Official agent integration uses `dv login` and the CLI; the Claude plugin records session transcripts, not an official operational MCP. Keep Git for this jam. [Official agent guide](https://docs.diversion.dev/agent-quickstart), [documentation index](https://docs.diversion.dev/llms.txt). |
| **Reallusion / iClone — community MCP; no verified official MCP** | Community `gorbabor/mc-iclone8-mcp` runs inside iClone 8 on Windows: `http://127.0.0.1:8766/mcp`, Streamable HTTP. Install its iClone plug-in and start it in the app; setup documents no MCP credential. App activation is separate. [Source](https://github.com/gorbabor/mc-iclone8-mcp), [Claude configuration](https://github.com/gorbabor/mc-iclone8-mcp/blob/main/docs/agent-configs.en.md). | Reallusion staff said official MCP was under consideration with no announced plan on **2026-10-02**. Local Python automation is not a cloud API credit allowance. Personal edition has 30 3D exports; AI Cloud entitlements are separate. Current Windows requirements list AMD support but do not certify the 8060S specifically. Defer this extra application unless essential. [Staff response](https://discussions.reallusion.com/t/the-official-reallusion-mcp/18463), [edition/requirements](https://www.reallusion.com/iclone/download.html). |
| **Rokoko — no verified official MCP** | No verified MCP URL/command. Official **Command API is ordinary HTTP**, not MCP: `http://{ip}:{port}/{version}/{apiKey}/{command}`; port defaults to `14053`, with a key configured in Studio. It controls the running app. [Official API documentation](https://support.rokoko.com/hc/en-us/articles/9738087323025-Rokoko-Studio-Command-API). | Create/Studio subscription allowances cover the documented motion workflow, not an established public generation MCP. Pricing lists Basic versus Advanced Command API access; advanced operations are Pro-tier. Use browser Create and Windows Studio exports to FBX/BVH; Basic is aimed at AI mocap without capture hardware. [Plans](https://www.rokoko.com/pricing), [Create/export workflow](https://create.rokoko.com/), [Windows Studio release](https://support.rokoko.com/hc/en-us/articles/18467538187409-Rokoko-Studio-2-4-2). |
| **WeLoveIndies — no verified public MCP or developer API** | No verified MCP URL/command/auth scheme. Website account and manual library licensing/downloads; ordinary Windows browser workflow. Downloads are WAV files. [Official service](https://www.weloveindies.com/en). | Subscription/licensing buys audio rights, not an established API credit balance. FREE & DEMO licensing covers demos and non-commercial releases; commercial release needs the appropriate commercial license. Keep the downloaded asset’s license record. [Official licensing summary](https://www.weloveindies.com/en). |

## Exact configuration reference for the parent

These are **unexecuted setup examples**, not instructions to install everything. Preserve the existing Unreal entry when merging configuration. Use a supported Node LTS satisfying the minimums above. Keep API values outside the repo and chat.

### Meshy plus optional fal

Merge the needed entries into the existing `mcpServers` object. Supply `MESHY_API_KEY` in the environment that launches Claude Code; `${MESHY_API_KEY}` below is a reference, not a credential. JSON configuration avoids PowerShell quoting ambiguity. [Meshy source](https://github.com/meshy-dev/meshy-mcp-server), [Claude environment expansion](https://code.claude.com/docs/en/mcp#environment-variable-expansion-in-mcpjson).

```json
{
  "mcpServers": {
    "meshy": {
      "type": "stdio",
      "command": "cmd",
      "args": ["/c", "npx", "-y", "@meshy-ai/meshy-mcp-server"],
      "env": { "MESHY_API_KEY": "${MESHY_API_KEY}" }
    },
    "fal": {
      "type": "http",
      "url": "https://mcp.fal.ai/mcp-relay"
    }
  }
}
```

Alternatively, add fal at user scope from PowerShell, then authenticate through `/mcp` → fal → Authenticate:

```powershell
claude mcp add --transport http --scope user fal https://mcp.fal.ai/mcp-relay
```

Do not configure both copies. First non-generating checks: Meshy `meshy_check_balance`; fal model search only. Use the current OAuth relay rather than older examples using `https://mcp.fal.ai/mcp` and static headers. [fal setup](https://fal.ai/docs/documentation/setting-up/mcp/claude-code), [current migration guidance](https://fal.ai/mcp).

### Tripo alternative, only with an API balance

The following adapts the official commands to Windows shims. `ov` is **only** for an international developer account; select `cn` instead for a mainland account.

```powershell
npm.cmd install -g tripo-cli
tripo.cmd login --region ov
tripo.cmd doctor
claude mcp add --transport stdio --scope user tripo "--" cmd /c tripo mcp
```

`doctor` checks connectivity/authentication/balance without submitting a generation. No need for the expanded toolset initially. [CLI reference](https://developers.tripo3d.ai/en/docs/cli).

### Hyper3D alternative

```powershell
claude mcp add --transport http --scope user hyper3d-rodin https://api.hyper3d.com/api/mcp
claude mcp login hyper3d-rodin
```

Select the intended billing workspace in the browser. `/mcp` authentication is also available in Claude Code. [Hyper3D setup](https://hyper3d.ai/features/mcp), [Claude OAuth CLI](https://code.claude.com/docs/en/mcp#authenticate-from-the-command-line).

## Verification limits

Negative MCP findings were checked against the linked vendor product/help pages, documentation index where available, and targeted public searches. Community repositories are primary sources for their own software, not vendor endorsements. No account-specific entitlement, Windows runtime compatibility, or generated-asset quality was tested. Recheck balances and active account/workspace during the parent’s setup; a web subscription alone is not a universal API entitlement.
