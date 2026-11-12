// Call one tool of the official Meshy MCP server from the shell, with the key from .claude/settings.local.json.
// Used when the agent session's own Meshy connection started without MESHY_API_KEY.
//   node tools/mcp/meshy-call.mjs <tool> [json-args | @args.json] [--timeout=seconds]
import { readFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { resolve } from 'node:path';
import { Client } from '@modelcontextprotocol/sdk/client/index.js';
import { StdioClientTransport } from '@modelcontextprotocol/sdk/client/stdio.js';

const root = fileURLToPath(new URL('../../', import.meta.url));
const config = JSON.parse(await readFile(resolve(root, '.mcp.json'), 'utf8')).mcpServers.meshy;
let localEnv = {};
try { localEnv = JSON.parse(await readFile(resolve(root, '.claude/settings.local.json'), 'utf8')).env ?? {}; } catch { /* none */ }
const key = localEnv.MESHY_API_KEY ?? process.env.MESHY_API_KEY ?? '';
if (!key) { console.error('MESHY_API_KEY not found'); process.exit(2); }

const positional = process.argv.slice(2).filter(a => !a.startsWith('--'));
const timeoutArg = process.argv.find(a => a.startsWith('--timeout='));
const timeoutMs = (timeoutArg ? Number(timeoutArg.split('=')[1]) : 330) * 1000;
const [tool, rawArgs = '{}'] = positional;
if (!tool) { console.error('usage: meshy-call.mjs <tool> [json | @file.json]'); process.exit(2); }
const args = JSON.parse(rawArgs.startsWith('@') ? await readFile(rawArgs.slice(1), 'utf8') : rawArgs);

const client = new Client({ name: 'megajam-meshy-call', version: '1.0' });
const transport = new StdioClientTransport({
  command: config.command, args: config.args, cwd: root,
  env: { ...process.env, MESHY_API_KEY: key, TRANSPORT: 'stdio' }, stderr: 'pipe'
});
transport.stderr?.on('data', () => {});
try {
  await client.connect(transport, { timeout: 15000 });
  const result = await client.callTool({ name: tool, arguments: args }, undefined, { timeout: timeoutMs, resetTimeoutOnProgress: true });
  for (const part of result.content ?? []) {
    if (part.type === 'text') console.log(part.text);
    else console.log(`[${part.type}]`);
  }
  if (result.isError) process.exitCode = 1;
} finally {
  await client.close();
}
