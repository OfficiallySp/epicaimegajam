// Read-only MCP checks. No Claude inference and no asset generation.
import { readFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { resolve } from 'node:path';
import { Client } from '@modelcontextprotocol/sdk/client/index.js';
import { StdioClientTransport } from '@modelcontextprotocol/sdk/client/stdio.js';
import { StreamableHTTPClientTransport } from '@modelcontextprotocol/sdk/client/streamableHttp.js';

const root = fileURLToPath(new URL('../../', import.meta.url));
const config = JSON.parse(await readFile(resolve(root, '.mcp.json'), 'utf8'));
let localEnv = {};
try {
  localEnv = JSON.parse(await readFile(resolve(root, '.claude/settings.local.json'), 'utf8')).env ?? {};
} catch (error) {
  if (error.code !== 'ENOENT') throw error;
}
const env = { ...process.env, ...localEnv };
const expand = value => value.replace(/\$\{([^}:]+)(?::-([^}]*))?\}/g,
  (_, name, fallback) => env[name] ?? fallback ?? '');
const requested = process.argv.slice(2).filter(arg => !arg.startsWith('--'));

for (const [name, entry] of Object.entries(config.mcpServers)) {
  if (requested.length && !requested.includes(name)) continue;
  // Hosted-service credentials live in Claude's OAuth store; do not copy them here.
  if (name === 'fal' || name === 'hyper3d-rodin') {
    try {
      const response = await fetch(entry.url, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json', Accept: 'application/json, text/event-stream' },
        body: JSON.stringify({ jsonrpc: '2.0', id: 1, method: 'initialize', params: {
          protocolVersion: '2025-03-26', capabilities: {}, clientInfo: { name: 'megajam-setup-check', version: '1.0' }
        } }),
        signal: AbortSignal.timeout(10000)
      });
      console.log(`${name}: endpoint HTTP ${response.status}; check saved OAuth status with 'claude mcp get ${name}'.`);
      await response.body?.cancel();
    } catch {
      console.log(`${name}: endpoint unreachable from this process.`);
      process.exitCode = 1;
    }
    continue;
  }
  const client = new Client({ name: 'megajam-setup-check', version: '1.0' });
  const transport = entry.type === 'stdio'
    ? new StdioClientTransport({
        command: entry.command,
        args: entry.args,
        cwd: root,
        env: Object.fromEntries(Object.entries(entry.env ?? {}).map(([key, value]) => [key, expand(value)])),
        stderr: 'pipe'
      })
    : new StreamableHTTPClientTransport(new URL(entry.url), {
        requestInit: { signal: AbortSignal.timeout(10000) }
      });
  // Drain diagnostics without exposing server responses or credentials.
  if (entry.type === 'stdio') transport.stderr?.on('data', () => {});
  try {
    await client.connect(transport, { timeout: 10000 });
    const result = await client.listTools({}, { timeout: 10000 });
    console.log(`${name}: MCP handshake OK; ${result.tools.length} tools available.`);
    if (name === 'meshy') {
      const hasKey = Boolean(expand(entry.env.MESHY_API_KEY).trim());
      console.log(`meshy: API key ${hasKey ? 'present (not validated by tool listing)' : 'missing; account calls need MESHY_API_KEY'}.`);
      if (hasKey && process.argv.includes('--balance')) {
        const balance = await client.callTool({ name: 'meshy_check_balance', arguments: {} }, undefined, { timeout: 15000 });
        console.log(`meshy: read-only balance call ${balance.isError ? 'FAILED' : 'OK'}.`);
        if (balance.isError) {
          const message = JSON.stringify(balance.content);
          const reason = /401|authentication failed/i.test(message) ? 'API rejected the key'
            : /environment variable is required/i.test(message) ? 'API key did not reach the server'
            : /EACCES|EPERM|network|connect|ENOTFOUND|timeout|timed out/i.test(message) ? 'network or request failure'
            : 'API/server error; inspect in the client';
          console.log(`meshy: ${reason}.`);
          process.exitCode = 1;
        }
      }
    }
  } catch {
    console.log(`${name}: connection failed.${name === 'unreal-mcp' ? ' Open the editor and enable Unreal MCP + All Toolsets and Auto Start Server.' : ''}`);
    process.exitCode = 1;
  } finally {
    await client.close();
  }
}
