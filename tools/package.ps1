<#
.SYNOPSIS
    Package the project for Win64 with UAT BuildCookRun. Development config by default (the jam requires a development build).
.DESCRIPTION
    Output:  dist\Win64\Windows\  (the folder to zip: <Project>.exe + Engine\ + <Project>\)
    Log:     dist\logs\package-<timestamp>.log (UAT's own log: %APPDATA%\Unreal Engine\AutomationTool\Logs\...)
    Maps:    Project Settings > Packaging > "List of maps to include in a packaged build", or -Maps.
    Flags:   pak + IO store, compressed, no debug symbols (-nodebuginfo), prerequisites installer included.
    If UnrealEditor.exe is running, the editor target is not rebuilt; close the editor after C++ changes.
.EXAMPLE
    .\tools\package.ps1
    .\tools\package.ps1 -Clean -Maps L_MainMenu,L_Level01
#>
[CmdletBinding()]
param(
    [string]$EngineRoot,
    [ValidateSet('Development', 'DebugGame', 'Shipping')]
    [string]$Config = 'Development',
    [string[]]$Maps,
    [string]$OutDir,
    [switch]$Clean,
    [switch]$SkipBuildEditor,
    [switch]$IncludeDebugInfo,
    [switch]$NoPrereqs
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')
Import-JamEnv

if (-not $EngineRoot) { $EngineRoot = Get-JamSetting 'UE_ROOT_WIN' 'C:\Program Files\Epic Games\UE_5.8' }
$runUat = Join-Path $EngineRoot 'Engine\Build\BatchFiles\RunUAT.bat'
if (-not (Test-Path $runUat)) { throw "RunUAT.bat not found at $runUat. Set UE_ROOT_WIN in tools\jam.env or pass -EngineRoot." }

$project = Get-UProject
if (-not $OutDir) { $OutDir = Join-Path $RepoRoot 'dist\Win64' }
$logDir = Join-Path $RepoRoot 'dist\logs'
if ($Clean -and (Test-Path $OutDir)) { Write-Step "Removing $OutDir"; Remove-Item -Recurse -Force $OutDir }
New-Item -ItemType Directory -Force -Path $OutDir, $logDir | Out-Null
$log = Join-Path $logDir ('package-{0}.log' -f (Get-Date -Format 'yyyyMMdd-HHmmss'))

if (-not $SkipBuildEditor -and (Get-Process -Name UnrealEditor -ErrorAction SilentlyContinue)) {
    Write-Warning 'UnrealEditor.exe is running, so the editor target will not be rebuilt (-skipbuildeditor). If C++ changed since the last full build, close the editor and run again.'
    $SkipBuildEditor = $true
}

$uatArgs = @(
    'BuildCookRun'
    "-project=$($project.FullName)"
    "-target=$($project.BaseName)"
    '-platform=Win64'
    "-clientconfig=$Config"
    '-build', '-cook', '-stage', '-pak', '-iostore', '-compressed', '-package', '-archive'
    "-archivedirectory=$OutDir"
    '-nop4', '-utf8output', '-unattended', '-installed', '-nocompile', '-nocompileuat'
)
if (-not $IncludeDebugInfo) { $uatArgs += '-nodebuginfo' }
if (-not $NoPrereqs) { $uatArgs += '-prereqs' }
if ($SkipBuildEditor) { $uatArgs += '-skipbuildeditor', '-nocompileeditor' }
if ($Maps) { $uatArgs += "-map=$($Maps -join '+')" }

Write-Step "$($project.BaseName): $Config Win64 -> $OutDir"
Write-Host "    RunUAT $($uatArgs -join ' ')"
Write-Host "    log: $log"
$stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
$transcript = $false
try { Start-Transcript -Path $log -ErrorAction Stop | Out-Null; $transcript = $true }
catch { Write-Warning "No transcript: $($_.Exception.Message)" }
try {
    & $runUat @uatArgs
    $exitCode = $LASTEXITCODE
} finally {
    if ($transcript) { Stop-Transcript | Out-Null }
}
$stopwatch.Stop()
if ($exitCode -ne 0) {
    throw ("BuildCookRun failed (exit {0}) after {1:hh\:mm\:ss}. Search for 'Error:' in {2}" -f $exitCode, $stopwatch.Elapsed, $log)
}

# UE 5.8 archives straight into the archive directory; older versions add a Windows\ folder.
$staged = Join-Path $OutDir 'Windows'
if (-not (Test-Path $staged)) { $staged = $OutDir }
$files = @(Get-ChildItem $staged -Recurse -File)
$bytes = ($files | Measure-Object Length -Sum).Sum
$exe = Get-ChildItem $staged -Filter *.exe -File | Select-Object -First 1
Write-Step ('Packaged in {0:hh\:mm\:ss}: {1} ({2} files, {3}); launcher {4}' -f $stopwatch.Elapsed, $staged, $files.Count, (Format-Size $bytes), $exe.Name)
if ($bytes -gt 950MB) { Write-Warning 'The staged build is already near the 1 GB zip limit. Trim the maps to cook, texture sizes and unused packs before zipping.' }
Write-Host 'Next: .\tools\zip-check.ps1  (zips to dist\TeamName_GameName.zip and checks the jam limits)'
