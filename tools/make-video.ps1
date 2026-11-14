<#
.SYNOPSIS
    Record the gameplay video from the packaged build, hands-free: four scenes played with real mouse input, cut together.
.DESCRIPTION
    Each clip launches the packaged game, records its window with ffmpeg and its audio with the in-game F9 capture, plays
    one scene through tools\drive-game.ps1, then muxes picture and sound. The clips are joined with hard cuts.
    Do not touch the mouse or keyboard while it runs (about 3 minutes). Output: dist\video\FixItInPost_gameplay.mp4
.EXAMPLE
    .\tools\make-video.ps1
    .\tools\make-video.ps1 -AudioLead 0.7     # seconds the audio capture starts before the picture
#>
[CmdletBinding()]
param([double]$AudioLead = 0.8, [string]$Out = 'dist\video\FixItInPost_gameplay.mp4')
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')
$work = Join-Path $RepoRoot '.cache\video'
New-Item -ItemType Directory -Force -Path $work, (Split-Path (Join-Path $RepoRoot $Out)) | Out-Null
$wavPath = Join-Path $RepoRoot 'dist\Win64\MegaJam26\Saved\BouncedWavFiles\MJCapture.wav'
$drive = Join-Path $PSScriptRoot 'drive-game.ps1'

# name, level ('' = title screen), seconds, steps after the recording has started. Coordinates: SMJHud's 1920x1080 canvas.
# Clip a: PLAY opens the cutting room under the REEL 1 card (2.6 s); SPACE rolls the untouched studio cut, which fails;
# FIX IT IN POST, cut the soda advert, roll again.
$clips = @(
    @{ Name = 'a-product-placement'; Level = ''; Seconds = 19; Fresh = $true
       Steps = "wait 2.2; click 276 742; wait 3; key space; wait 3.6; click 960 650; wait 1.2; click 516 945; wait 0.6; key x; wait 1.2; key space; wait 6.5" },
    @{ Name = 'b-hold-the-door'; Level = 'r2s1'; Seconds = 12
       Steps = "wait 1.6; drag 332 1023 516 1023; wait 0.9; key space; wait 7" },
    @{ Name = 'c-the-monologue'; Level = 'r3s2'; Seconds = 13
       Steps = "wait 1.6; drag 1718 877 250 1023; wait 0.9; key space; wait 8" },
    @{ Name = 'd-hit-and-run'; Level = 'r3s3'; Seconds = 14
       Steps = "wait 1.6; drag 1718 799 650 1023; wait 0.9; key space; wait 9" }
)

$parts = @()
foreach ($c in $clips) {
    $video = Join-Path $work "$($c.Name).video.mp4"
    $final = Join-Path $work "$($c.Name).mp4"
    $rel = $video.Substring($RepoRoot.Length + 1)
    $steps = "wait 2; key f9; record $rel $($c.Seconds); $($c.Steps); key f9; wait 1.5"
    Write-Step "clip $($c.Name)"
    $driveArgs = @{ Steps = $steps }
    if ($c.Level) { $driveArgs.Level = $c.Level }
    if ($c.Fresh) { $driveArgs.FreshSave = $true }
    & $drive @driveArgs
    Start-Sleep -Seconds 2
    if (-not (Test-Path $video)) { throw "no video for $($c.Name)" }
    # screen capture needs a live display: with the monitors off (or a locked session) every frame is black
    $luma = (& ffmpeg -hide_banner -ss 2 -i $video -frames:v 1 -vf "signalstats,metadata=print:key=lavfi.signalstats.YAVG" -f null - 2>&1 | Select-String 'YAVG=' | Select-Object -First 1) -replace '.*YAVG=', ''
    if ([double]$luma -lt 22) { Get-Process MegaJam26* -ErrorAction SilentlyContinue | Stop-Process -Force; throw "The recording of $($c.Name) is black (mean luma $luma). Turn the monitor on, unlock the session and run this again." }
    & ffmpeg -y -hide_banner -loglevel error -i $video -ss $AudioLead -i $wavPath -map 0:v -map 1:a -c:v libx264 -preset slow -crf 18 -pix_fmt yuv420p -r 60 -c:a aac -b:a 192k -ar 48000 -shortest $final
    if ($LASTEXITCODE -ne 0) { throw "mux failed for $($c.Name)" }
    $parts += $final
}

$list = Join-Path $work 'concat.txt'
$parts | ForEach-Object { "file '$($_ -replace '\\', '/')'" } | Set-Content -Path $list -Encoding ascii
$outPath = Join-Path $RepoRoot $Out
& ffmpeg -y -hide_banner -loglevel error -f concat -safe 0 -i $list -c:v libx264 -preset slow -crf 18 -pix_fmt yuv420p -c:a aac -b:a 192k -movflags +faststart $outPath
if ($LASTEXITCODE -ne 0) { throw 'concat failed' }
$dur = & ffprobe -v error -show_entries format=duration -of csv=p=0 $outPath
Write-Step ("{0}: {1:N1} s, {2:N1} MB" -f $outPath, [double]$dur, ((Get-Item $outPath).Length / 1MB))
if ([double]$dur -lt 30 -or [double]$dur -gt 60) { Write-Warning 'The jam asks for a 30 to 60 second video.' }
