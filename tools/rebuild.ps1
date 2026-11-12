<#
.SYNOPSIS
    Close this project's editor, run a full editor build, reopen the editor and wait for Unreal MCP.
.DESCRIPTION
    Used after header changes (new UPROPERTY / UFUNCTION / members), which Live Coding cannot patch.
    Only the editor whose command line names this project's .uproject is closed; other Unreal projects are left alone.
    -NoHotReloadFromIDE skips UBT's "Live Coding is active" guard, which is keyed on UnrealEditor.exe and would
    otherwise be tripped by any other open Unreal project.
.EXAMPLE
    .\tools\rebuild.ps1            # close, build, reopen
    .\tools\rebuild.ps1 -NoOpen    # close and build only
#>
[CmdletBinding()]
param([switch]$NoOpen, [int]$McpPort = 8000)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')
Import-JamEnv
$engineRoot = Get-JamSetting 'UE_ROOT_WIN'
$project = Get-UProject
$cache = Join-Path $RepoRoot '.cache'
New-Item -ItemType Directory -Force -Path $cache | Out-Null
$log = Join-Path $cache 'build.log'

$mine = Get-CimInstance Win32_Process -Filter "Name like 'UnrealEditor%'" | Where-Object { $_.CommandLine -like "*$($project.Name)*" }
foreach ($p in $mine) { Stop-Process -Id $p.ProcessId -Force -ErrorAction SilentlyContinue }
if ($mine) { Start-Sleep -Seconds 3 }

$ErrorActionPreference = 'Continue'
& (Join-Path $engineRoot 'Engine\Build\BatchFiles\Build.bat') "$($project.BaseName)Editor" Win64 Development "-Project=$($project.FullName)" -WaitMutex -NoHotReloadFromIDE *> $log
$code = $LASTEXITCODE
Select-String -Path $log -Pattern ' error |error C|error LNK|fatal error|Result:|Total execution|warning C' | Select-Object -First 60 | ForEach-Object { $_.Line }
if ($code -ne 0) { Write-Host "BUILD FAILED (exit $code). Full log: $log"; exit $code }

if (-not $NoOpen) {
    & (Join-Path $PSScriptRoot 'open-editor.ps1')
    $deadline = (Get-Date).AddSeconds(300)
    $ok = $false
    while ((Get-Date) -lt $deadline) {
        try { $c = New-Object System.Net.Sockets.TcpClient; $c.Connect('127.0.0.1', $McpPort); $c.Close(); $ok = $true; break } catch { Start-Sleep -Seconds 3 }
    }
    Start-Sleep -Seconds 12   # let the startup scripts and toolset registration finish
    Write-Host "Editor MCP port open: $ok"
}
