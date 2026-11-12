<#
.SYNOPSIS
    Copy the newest editor/PIE screenshot (console command `shot showui`) to a stable path and print it.
.EXAMPLE
    .\tools\latest-shot.ps1                      # -> .cache\shot.png
    .\tools\latest-shot.ps1 -Out devlog\img\first-playable.png
#>
[CmdletBinding()]
param([string]$Out = '.cache\shot.png', [int]$WaitSeconds = 6)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')
$dir = Join-Path $RepoRoot 'Saved\Screenshots'
$deadline = (Get-Date).AddSeconds($WaitSeconds)
$file = $null
do {
    $file = Get-ChildItem -Recurse $dir -File -Filter *.png -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if ($file -and $file.LastWriteTime -gt (Get-Date).AddSeconds(-20)) { break }
    Start-Sleep -Milliseconds 400
} while ((Get-Date) -lt $deadline)
if (-not $file) { throw 'No screenshot found.' }
Start-Sleep -Milliseconds 300
$dest = Join-Path $RepoRoot $Out
New-Item -ItemType Directory -Force -Path (Split-Path $dest) | Out-Null
Copy-Item $file.FullName $dest -Force
Write-Host "$dest  (from $($file.Name), $($file.LastWriteTime.ToString('HH:mm:ss')))"
