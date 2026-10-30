# Shared helpers for the jam scripts. Dot-source from a script:
#     . (Join-Path $PSScriptRoot 'common.ps1')
# Provides: $RepoRoot, Import-JamEnv, Get-JamSetting, Get-JamZipBaseName, Get-UProject, Format-Size, Write-Step

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

function Import-JamEnv {
    # Load KEY=VALUE lines from tools\jam.env into the process environment. Existing environment variables win.
    $file = Join-Path $PSScriptRoot 'jam.env'
    if (-not (Test-Path $file)) { return }
    foreach ($line in Get-Content $file) {
        if ($line -match '^\s*(#|$)') { continue }
        if ($line -match '^\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.*?)\s*$') {
            $key = $Matches[1]
            $value = $Matches[2].Trim().Trim('"').Trim("'")
            if ($value -ne '' -and -not [Environment]::GetEnvironmentVariable($key)) {
                [Environment]::SetEnvironmentVariable($key, $value)
            }
        }
    }
}

function Get-JamSetting([string]$Name, [string]$Default = '') {
    $value = [Environment]::GetEnvironmentVariable($Name)
    if ($value) { return $value }
    return $Default
}

function Get-JamZipBaseName {
    # TeamName_GameName with everything but letters and digits removed (the jam's required zip name).
    $team = (Get-JamSetting 'JAM_TEAM_NAME') -replace '[^A-Za-z0-9]', ''
    $game = (Get-JamSetting 'JAM_GAME_NAME') -replace '[^A-Za-z0-9]', ''
    if (-not $team -or -not $game) {
        throw 'Set JAM_TEAM_NAME and JAM_GAME_NAME in tools\jam.env (or pass -TeamName/-GameName). Only letters and digits survive.'
    }
    return "${team}_${game}"
}

function Get-UProject {
    $project = Get-ChildItem -Path $RepoRoot -Filter *.uproject -File | Select-Object -First 1
    if (-not $project) { throw "No .uproject found in $RepoRoot (the repo root must be the Unreal project root)." }
    return $project
}

function Format-Size([long]$Bytes) {
    if ($Bytes -ge 1GB) { return ('{0:N2} GiB' -f ($Bytes / 1GB)) }
    if ($Bytes -ge 1MB) { return ('{0:N1} MiB' -f ($Bytes / 1MB)) }
    return ('{0:N0} KiB' -f ($Bytes / 1KB))
}

function Write-Step([string]$Text) {
    Write-Host "==> $Text" -ForegroundColor Cyan
}
