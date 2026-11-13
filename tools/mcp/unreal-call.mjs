// Call one Unreal MCP toolset tool from the shell (the editor must be open; one client and one call at a time).
// Used when the agent session's own unreal-mcp connection is down, and for calls whose output is filtered in the shell.
//   node tools/mcp/unreal-call.mjs <toolset> <tool> [json-args | @args.json]
//   node tools/mcp/unreal-call.mjs py "@script.py"          shorthand: run editor Python through MJDevTools.run_python
//   node tools/mcp/unreal-call.mjs console "mj.Dump"        shorthand: console command (PIE world if running)
//   node tools/mcp/unreal-call.mjs pie start|stop           shorthand: Play-In-Editor in a floating window
import { readFile } from 'node:fs/promises';
import { Client } from '@modelcontextprotocol/sdk/client/index.js';
import { StreamableHTTPClientTransport } from '@modelcontextprotocol/sdk/client/streamableHttp.js';

const url = process.env.UNREAL_MCP_URL ?? 'http://127.0.0.1:8000/mcp';
const [a, b, c] = process.argv.slice(2);
const load = async v => (v && v.startsWith('@') ? await readFile(v.slice(1), 'utf8') : v);

let toolset, tool, args;
if (a === 'py') { toolset = 'mj_devtools.MJDevTools'; tool = 'run_python'; args = { code: await load(b) }; }
else if (a === 'console') { toolset = 'mj_devtools.MJDevTools'; tool = 'console'; args = { command: b }; }
else if (a === 'pie') {
  toolset = 'EditorToolset.EditorAppToolset';
  tool = b === 'stop' ? 'StopPIE' : 'StartPIE';
  args = b === 'stop' ? {} : { options: { bSimulate: false, playMode: 'PlayMode_InEditorFloating', warmupSeconds: Number(c ?? 6) } };
}
else if (a && b) { toolset = a; tool = b; args = JSON.parse((await load(c)) ?? '{}'); }
else { console.error('usage: unreal-call.mjs <toolset> <tool> [json | @file] | py <code | @file> | console <cmd> | pie start|stop'); process.exit(2); }

const client = new Client({ name: 'megajam-unreal-call', version: '1.0' });
const transport = new StreamableHTTPClientTransport(new URL(url));
try {
  await client.connect(transport, { timeout: 15000 });
  const result = await client.callTool({ name: 'call_tool', arguments: { toolset_name: toolset, tool_name: tool, arguments: args } }, undefined, { timeout: 600000 });
  for (const part of result.content ?? []) {
    if (part.type !== 'text') { console.log(`[${part.type}]`); continue; }
    let text = part.text;
    try { const parsed = JSON.parse(text); if (typeof parsed?.returnValue === 'string') text = parsed.returnValue; } catch { /* not JSON */ }
    console.log(text);
  }
  if (result.isError) process.exitCode = 1;
} catch (error) {
  console.error(`unreal-call failed: ${error.message}`);
  process.exitCode = 1;
} finally {
  await client.close().catch(() => {});
}
