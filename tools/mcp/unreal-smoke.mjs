// Runs a ten-second PIE session in the open editor. No model calls or saved game assets.
// Do not run another MCP client against this editor while this check is active.
import { Client } from '@modelcontextprotocol/sdk/client/index.js';
import { StreamableHTTPClientTransport } from '@modelcontextprotocol/sdk/client/streamableHttp.js';
import { readFile, writeFile, mkdir } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { resolve } from 'node:path';

const root = fileURLToPath(new URL('../../', import.meta.url));
const config = JSON.parse(await readFile(resolve(root, '.mcp.json'), 'utf8'));
const client = new Client({ name: 'megajam-unreal-smoke', version: '1.0' });
const editor = 'EditorToolset.EditorAppToolset';
const logPath = resolve(root, 'Saved/Logs/MegaJam26.log');
const report = { checkedAt: new Date().toISOString(), steps: [] };
let startedPlay = false;

function unpack(result) {
  const text = result.content?.filter(item => item.type === 'text').map(item => item.text).join('\n') ?? '';
  if (result.isError) throw new Error(text);
  if (result.structuredContent) return result.structuredContent;
  try { return JSON.parse(text); } catch { return text; }
}

async function call(toolset, tool, args = {}) {
  const result = unpack(await client.callTool({
    name: 'call_tool',
    arguments: { toolset_name: toolset, tool_name: tool, arguments: args }
  }, undefined, { timeout: 60000 }));
  report.steps.push({ tool: `${toolset}.${tool}`, result });
  console.log(`${tool}: ${JSON.stringify(result)}`);
  return result;
}

try {
  await client.connect(new StreamableHTTPClientTransport(new URL(config.mcpServers['unreal-mcp'].url)));
  const tools = await client.listTools();
  console.log(`MCP connected: ${tools.tools.map(tool => tool.name).join(', ')}`);
  report.toolsets = unpack(await client.callTool({ name: 'list_toolsets', arguments: {} }));
  await call(editor, 'GetSelectedActors');
  const before = await call(editor, 'IsPIERunning');
  if (before.returnValue !== false) throw new Error('PIE is already running, or state could not be confirmed; leaving it untouched.');
  const baseline = (await readFile(logPath, 'utf8')).split('\n').length - 1;
  // Mark before requesting so cleanup also runs if PIE starts but its response fails.
  startedPlay = true;
  await call(editor, 'StartPIE', {
    options: { bSimulate: false, playMode: 'PlayMode_InViewPort', warmupSeconds: 10 }
  });
  const during = await call(editor, 'IsPIERunning');
  if (during.returnValue !== true) throw new Error('PIE did not remain active after the ten-second warmup.');
  await call(editor, 'StopPIE');
  startedPlay = false;
  const after = await call(editor, 'IsPIERunning');
  if (after.returnValue !== false) throw new Error('PIE did not stop.');
  await call('ConsoleToolset.LogsToolset', 'GetLogEntries', {
    logCategory: 'LogPlayLevel', pattern: '.', maxEntries: 12
  });
  const newLines = (await readFile(logPath, 'utf8')).split('\n').slice(baseline);
  report.errors = newLines.filter(line => /\]Log[^:]+:\s*(Error|Fatal):/.test(line));
  report.warnings = newLines.filter(line => /\]Log[^:]+:\s*Warning:/.test(line));
  report.passed = report.errors.length === 0;
  console.log(`PIE interval: ${report.errors.length} Error/Fatal lines, ${report.warnings.length} warnings.`);
  for (const line of [...report.errors, ...report.warnings]) console.log(line);
  if (!report.passed) process.exitCode = 1;
} catch (error) {
  report.passed = false;
  report.failure = error.message;
  console.error(error.message);
  process.exitCode = 1;
} finally {
  if (startedPlay) {
    try {
      const state = await call(editor, 'IsPIERunning');
      if (state.returnValue === true) await call(editor, 'StopPIE');
    } catch (error) { console.error(`PIE cleanup needs attention: ${error.message}`); }
  }
  await client.close();
  await mkdir(resolve(root, '.cache'), { recursive: true });
  await writeFile(resolve(root, '.cache/unreal-smoke.json'), JSON.stringify(report, null, 2) + '\n');
}
