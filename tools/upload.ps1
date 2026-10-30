<#
.SYNOPSIS
    Push a build to itch.io with butler (iteration builds and backups).
.DESCRIPTION
    Target and channel come from tools\jam.env (ITCH_TARGET=user/game-slug, ITCH_CHANNEL=win64-dev) or parameters.
    Pushes dist\TeamName_GameName.zip if it exists, otherwise dist\Win64\Windows. User version = date-time-gitsha.
    One-time setup: butler login. The FINAL submission zip goes through the itch web dashboard so the filename
    stays exactly TeamName_GameName.zip (see docs/submission-checklist.md).
.EXAMPLE
    .\tools\upload.ps1
    .\tools\upload.ps1 -Path .\dist\Win64\Windows -Channel win64-nightly -DryRun
#>
[CmdletBinding()]
param(
    [string]$Path,
    [string]$Target,
    [string]$Channel,
    [string]$UserVersion,
    [switch]$DryRun
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')
Import-JamEnv

if (-not (Get-Command butler -ErrorAction SilentlyContinue)) {
    throw 'butler is not on PATH. Install: https://itch.io/docs/butler/installing.html, then run: butler login'
}
if (-not $Target) { $Target = Get-JamSetting 'ITCH_TARGET' }
if ($Target -notmatch '^[a-z0-9-]+/[a-z0-9-]+$') { throw "ITCH_TARGET must be user/game-slug (tools\jam.env). Got: '$Target'" }
if (-not $Channel) { $Channel = Get-JamSetting 'ITCH_CHANNEL' 'win64-dev' }

if (-not $Path) {
    $zipBase = $null
    try { $zipBase = Get-JamZipBaseName } catch { $zipBase = $null }
    $zipPath = if ($zipBase) { Join-Path $RepoRoot "dist\$zipBase.zip" } else { $null }
    if ($zipPath -and (Test-Path $zipPath)) { $Path = $zipPath } else { $Path = Join-Path $RepoRoot 'dist\Win64\Windows' }
}
if (-not (Test-Path $Path)) { throw "Nothing to upload at $Path. Run package.ps1 and zip-check.ps1 first." }

if (-not $UserVersion) {
    $sha = ''
    if (Get-Command git -ErrorAction SilentlyContinue) {
        $sha = & { $ErrorActionPreference = 'Continue'; git -C $RepoRoot rev-parse --short HEAD 2>$null }
    }
    $UserVersion = Get-Date -Format 'yyyyMMdd-HHmm'
    if ($sha) { $UserVersion += "-$sha" }
}

$pushArgs = @('push', $Path, "${Target}:${Channel}", '--userversion', $UserVersion)
if ($DryRun) { $pushArgs += '--dry-run' }
Write-Step "butler $($pushArgs -join ' ')"
& butler @pushArgs
if ($LASTEXITCODE -ne 0) { throw "butler push failed (exit $LASTEXITCODE). Not logged in? Run: butler login" }

Write-Step "Channel status for ${Target}:${Channel}"
& butler status "${Target}:${Channel}"
$user, $game = $Target.Split('/')
Write-Host ''
Write-Host "Now open https://$user.itch.io/$game, download the build, extract it into a NEW folder and run it."
Write-Host 'Check the downloaded filename: the final submission must be TeamName_GameName.zip (docs/submission-checklist.md).'
