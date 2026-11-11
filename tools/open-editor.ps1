# Open the installed Unreal editor with this repository's project and MCP defaults.
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')
Import-JamEnv
$engineRoot = Get-JamSetting 'UE_ROOT_WIN'
$editorPath = Join-Path $engineRoot 'Engine\Binaries\Win64\UnrealEditor.exe'
if (-not (Test-Path -LiteralPath $editorPath)) {
    throw 'UnrealEditor.exe not found. Set UE_ROOT_WIN in tools/jam.env.'
}
$project = Get-UProject
$editorProcess = Start-Process -FilePath $editorPath -ArgumentList @('"' + $project.FullName + '"', '-NoSplash') -WorkingDirectory $RepoRoot -PassThru
Write-Host "Opening $($project.Name) (PID $($editorProcess.Id)). Unreal MCP starts on localhost:8000."
