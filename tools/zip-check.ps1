<#
.SYNOPSIS
    Zip a packaged Win64 build as TeamName_GameName.zip and check it against the jam rules.
.DESCRIPTION
    Names come from tools\jam.env (JAM_TEAM_NAME, JAM_GAME_NAME) or -TeamName/-GameName.
    Zips the contents of dist\Win64\Windows with the exe at the zip root (*.pdb excluded), then checks:
    the name pattern, size under 1 GB (1,000,000,000 bytes), an .exe at the root, an Engine\ folder.
    Exit code 1 on any failure. Use -CheckOnly -ZipPath <file> to check a zip you downloaded from itch.
.EXAMPLE
    .\tools\zip-check.ps1
    .\tools\zip-check.ps1 -CheckOnly -ZipPath "$env:USERPROFILE\Downloads\TeamName_GameName.zip"
#>
[CmdletBinding()]
param(
    [string]$TeamName,
    [string]$GameName,
    [string]$BuildDir,
    [string]$ZipPath,
    [long]$MaxBytes = 1000000000,
    [switch]$CheckOnly
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')
Import-JamEnv
if ($TeamName) { $env:JAM_TEAM_NAME = $TeamName }
if ($GameName) { $env:JAM_GAME_NAME = $GameName }

$problems = New-Object System.Collections.Generic.List[string]
$warnings = New-Object System.Collections.Generic.List[string]

if (-not $CheckOnly) {
    $zipBase = Get-JamZipBaseName
    if (-not $BuildDir) {
        # UE 5.8 archives straight into dist\Win64; older versions add a Windows\ folder.
        $BuildDir = Join-Path $RepoRoot 'dist\Win64\Windows'
        if (-not (Test-Path $BuildDir)) { $BuildDir = Join-Path $RepoRoot 'dist\Win64' }
    }
    if (-not $ZipPath) { $ZipPath = Join-Path $RepoRoot "dist\$zipBase.zip" }
    if (-not (Test-Path $BuildDir)) { throw "Build folder not found: $BuildDir. Run .\tools\package.ps1 first." }

    $files = @(Get-ChildItem $BuildDir -Recurse -File)
    $sourceBytes = ($files | Measure-Object Length -Sum).Sum
    $rootExe = @(Get-ChildItem $BuildDir -Filter *.exe -File)
    if ($rootExe.Count -eq 0) { $problems.Add("No .exe at the top level of $BuildDir") }
    $pdbs = @($files | Where-Object { $_.Extension -eq '.pdb' })
    if ($pdbs.Count -gt 0) {
        $pdbBytes = ($pdbs | Measure-Object Length -Sum).Sum
        $warnings.Add("$($pdbs.Count) .pdb file(s) ($(Format-Size $pdbBytes)) in the build folder; package.ps1 uses -nodebuginfo so these should not be there")
    }
    Write-Step "Build folder $BuildDir : $($files.Count) files, $(Format-Size $sourceBytes)"

    if (Test-Path $ZipPath) { Remove-Item $ZipPath -Force }
    New-Item -ItemType Directory -Force -Path (Split-Path $ZipPath -Parent) | Out-Null
    $sevenZip = $null
    $cmd = Get-Command 7z.exe -ErrorAction SilentlyContinue
    if ($cmd) { $sevenZip = $cmd.Source }
    elseif (Test-Path 'C:\Program Files\7-Zip\7z.exe') { $sevenZip = 'C:\Program Files\7-Zip\7z.exe' }

    $stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
    if ($sevenZip) {
        Write-Step "7-Zip -> $ZipPath"
        & $sevenZip a -tzip -mx=5 -r '-xr!*.pdb' '-xr!Saved' $ZipPath (Join-Path $BuildDir '*') | Out-Null   # Saved\ = logs and save games from local test runs
        if ($LASTEXITCODE -ne 0) { throw "7z failed with exit code $LASTEXITCODE" }
    } else {
        Write-Step "Compress-Archive -> $ZipPath (install 7-Zip for a faster zip that excludes *.pdb)"
        if ($pdbs.Count -gt 0) { $warnings.Add('Compress-Archive cannot exclude *.pdb; delete them from the build folder or install 7-Zip') }
        Compress-Archive -Path (Join-Path $BuildDir '*') -DestinationPath $ZipPath -CompressionLevel Optimal -Force
    }
    Write-Host ('    zipped in {0:hh\:mm\:ss}' -f $stopwatch.Elapsed)
}

if (-not $ZipPath) { throw 'Pass -ZipPath <file.zip> together with -CheckOnly.' }
if (-not (Test-Path $ZipPath)) { throw "Zip not found: $ZipPath" }
$zip = Get-Item $ZipPath
Write-Step "Checking $($zip.FullName)"

if ($zip.Name -notmatch '^[A-Za-z0-9]+_[A-Za-z0-9]+\.zip$') {
    $problems.Add("Name '$($zip.Name)' is not TeamName_GameName.zip (letters and digits, one underscore)")
}
if ($zip.Length -ge $MaxBytes) {
    $problems.Add("Size $(Format-Size $zip.Length) is not under the limit of $MaxBytes bytes ($(Format-Size $MaxBytes))")
} elseif ($zip.Length -ge [long]($MaxBytes * 0.9)) {
    $warnings.Add("Size $(Format-Size $zip.Length) is within 10% of the limit; trim maps, textures or packs (docs/submission-checklist.md)")
}

Add-Type -AssemblyName System.IO.Compression.FileSystem
$archive = [System.IO.Compression.ZipFile]::OpenRead($zip.FullName)
try {
    $entries = @($archive.Entries)
    $launchers = @($entries | Where-Object { $_.FullName -match '^[^/\\]+\.exe$' })
    if ($launchers.Count -eq 0) { $problems.Add('No .exe at the zip root. Judges expect: unzip, double-click the exe.') }
    $engine = @($entries | Where-Object { $_.FullName -match '^Engine[/\\]' })
    if ($engine.Count -eq 0) { $warnings.Add('No Engine/ folder at the zip root. Is this a UE packaged build?') }
    $pdbEntries = @($entries | Where-Object { $_.FullName -match '\.pdb$' })
    if ($pdbEntries.Count -gt 0) { $warnings.Add("$($pdbEntries.Count) .pdb file(s) inside the zip: wasted space") }
    $uncompressed = ($entries | Measure-Object Length -Sum).Sum
    $launcherNames = ($launchers | ForEach-Object { $_.FullName }) -join ', '
    Write-Host "    entries: $($entries.Count); uncompressed $(Format-Size $uncompressed); zip $(Format-Size $zip.Length); launcher: $launcherNames"
} finally {
    $archive.Dispose()
}

foreach ($w in $warnings) { Write-Warning $w }
if ($problems.Count -gt 0) {
    foreach ($p in $problems) { Write-Host "FAIL: $p" -ForegroundColor Red }
    exit 1
}
Write-Host "OK: $($zip.Name) ($(Format-Size $zip.Length)) passes the jam checks. Next: extract it into a NEW folder, run the exe, then .\tools\upload.ps1" -ForegroundColor Green
