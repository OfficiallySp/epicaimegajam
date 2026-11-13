<#
.SYNOPSIS
    Drive the packaged game with real mouse and keyboard input (end-to-end UI test, and the hands for capture runs).
.DESCRIPTION
    Starts dist\Win64\MegaJam26.exe windowed at 1600x900 and runs a small script of steps against it. Coordinates are the
    HUD's virtual 1920x1080 canvas (see SMJHud::ComputeLayout), mapped onto the window's client area.
    Steps (one per line or separated by ';'):
        wait <seconds>            click <x> <y>            drag <x1> <y1> <x2> <y2>
        key <name>                (space, x, z, y, r, h, esc, enter, left, right, f11)
        shot <file.png>           expect <regex>           (the regex must appear in the game log from now on)
.EXAMPLE
    .\tools\drive-game.ps1 -Steps "wait 6; click 276 742; wait 5; click 960 650; click 516 945; key x; key space; wait 5; expect 'PRINT IT! level r1s1'"
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$Steps,
    [string]$Level,
    [switch]$KeepOpen,
    [switch]$FreshSave,
    [switch]$Attach
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')
Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class MJIn {
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
    [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X, Y; }
    [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
    [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref POINT p);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int c);
    [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
    [DllImport("user32.dll")] public static extern void mouse_event(uint f, uint dx, uint dy, uint d, UIntPtr e);
    [DllImport("user32.dll")] public static extern void keybd_event(byte vk, byte scan, uint f, UIntPtr e);
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
    [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr h, IntPtr after, int x, int y, int cx, int cy, uint flags);
}
'@
[void][MJIn]::SetProcessDPIAware()

$exe = Join-Path $RepoRoot 'dist\Win64\MegaJam26.exe'
$saveDir = Join-Path $RepoRoot 'dist\Win64\MegaJam26\Saved'
$logFile = Join-Path $saveDir 'Logs\MegaJam26.log'
if (-not $Attach) {
    Get-Process MegaJam26* -ErrorAction SilentlyContinue | Stop-Process -Force
    if ($FreshSave) { Remove-Item (Join-Path $saveDir 'SaveGames\*.sav') -Force -ErrorAction SilentlyContinue }
    $gameArgs = @('-windowed', '-resx=1600', '-resy=900')
    if ($Level) { $gameArgs += "-MJLevel=$Level" }
    Start-Process -FilePath $exe -ArgumentList $gameArgs | Out-Null
    Start-Sleep -Seconds 12
}
$proc = Get-Process MegaJam26* | Where-Object { $_.MainWindowHandle -ne 0 } | Select-Object -First 1
if (-not $proc) { throw 'The game window did not appear.' }
$h = $proc.MainWindowHandle
[void][MJIn]::ShowWindow($h, 9)
[void][MJIn]::SetWindowPos($h, [IntPtr]::Zero, 60, 40, 0, 0, 0x0001 -bor 0x0004)   # move only, keep size and z-order
[void][MJIn]::SetForegroundWindow($h)
Start-Sleep -Milliseconds 600

function Get-Client {
    $r = New-Object MJIn+RECT; [void][MJIn]::GetClientRect($h, [ref]$r)
    $p = New-Object MJIn+POINT; [void][MJIn]::ClientToScreen($h, [ref]$p)
    [pscustomobject]@{ X = $p.X; Y = $p.Y; W = $r.Right - $r.Left; H = $r.Bottom - $r.Top }
}
function To-Screen([double]$vx, [double]$vy) {
    $c = Get-Client; $s = $c.H / 1080.0
    , @([int]($c.X + $vx * $s), [int]($c.Y + $vy * $s))
}
function Move-To([double]$vx, [double]$vy) { $p = To-Screen $vx $vy; [void][MJIn]::SetCursorPos($p[0], $p[1]) }
$keys = @{ space = 0x20; enter = 0x0D; esc = 0x1B; left = 0x25; right = 0x27; x = 0x58; z = 0x5A; y = 0x59; r = 0x52; h = 0x48; f = 0x46; f9 = 0x78; f11 = 0x7A; pgdn = 0x22; pgup = 0x21 }
$logMark = if (Test-Path $logFile) { (Get-Content $logFile).Count } else { 0 }
$failed = $false

foreach ($raw in ($Steps -split '[;\r\n]+')) {
    $step = $raw.Trim(); if (-not $step) { continue }
    $t = $step -split '\s+', 2; $verb = $t[0].ToLower(); $rest = if ($t.Count -gt 1) { $t[1] } else { '' }
    switch ($verb) {
        'wait' { Start-Sleep -Milliseconds ([int]([double]$rest * 1000)) }
        'click' {
            $a = $rest -split '\s+'; Move-To $a[0] $a[1]; Start-Sleep -Milliseconds 120
            [MJIn]::mouse_event(2, 0, 0, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 70; [MJIn]::mouse_event(4, 0, 0, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 350
        }
        'drag' {
            $a = ($rest -split '\s+') | ForEach-Object { [double]$_ }
            Move-To $a[0] $a[1]; Start-Sleep -Milliseconds 150
            [MJIn]::mouse_event(2, 0, 0, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 120
            for ($i = 1; $i -le 24; $i++) { $k = $i / 24.0; Move-To ($a[0] + ($a[2] - $a[0]) * $k) ($a[1] + ($a[3] - $a[1]) * $k); Start-Sleep -Milliseconds 28 }
            Start-Sleep -Milliseconds 200
            [MJIn]::mouse_event(4, 0, 0, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 400
        }
        'key' {
            $vk = $keys[$rest.ToLower()]; if (-not $vk) { throw "unknown key '$rest'" }
            [MJIn]::keybd_event([byte]$vk, 0, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 70; [MJIn]::keybd_event([byte]$vk, 0, 2, [UIntPtr]::Zero); Start-Sleep -Milliseconds 300
        }
        'shot' {
            $c = Get-Client; $bmp = New-Object System.Drawing.Bitmap $c.W, $c.H; $g = [System.Drawing.Graphics]::FromImage($bmp)
            $g.CopyFromScreen($c.X, $c.Y, 0, 0, $bmp.Size); $out = [System.IO.Path]::GetFullPath((Join-Path $RepoRoot $rest))
            New-Item -ItemType Directory -Force -Path (Split-Path $out) | Out-Null; $bmp.Save($out); $g.Dispose(); $bmp.Dispose(); Write-Host "shot $out"
        }
        'record' {
            # record <file.mp4> <seconds>: capture the game's client area (and system audio when MJ_AUDIO_DEVICE names a
            # DirectShow loopback device) in the background while the following steps run
            $a = $rest -split '\s+'; $c = Get-Client
            $out = [System.IO.Path]::GetFullPath((Join-Path $RepoRoot $a[0])); New-Item -ItemType Directory -Force -Path (Split-Path $out) | Out-Null
            $w = $c.W - ($c.W % 2); $hh = $c.H - ($c.H % 2)
            $ff = @('-y', '-hide_banner', '-loglevel', 'error', '-f', 'gdigrab', '-framerate', '60', '-draw_mouse', '1', '-offset_x', $c.X, '-offset_y', $c.Y, '-video_size', "${w}x${hh}", '-i', 'desktop')
            if ($env:MJ_AUDIO_DEVICE) { $ff += @('-f', 'dshow', '-i', ('audio="{0}"' -f $env:MJ_AUDIO_DEVICE)) }
            $ff += @('-t', $a[1], '-c:v', 'libx264', '-preset', 'veryfast', '-crf', '17', '-pix_fmt', 'yuv420p')
            if ($env:MJ_AUDIO_DEVICE) { $ff += @('-c:a', 'aac', '-b:a', '192k') }
            $ff += ('"{0}"' -f $out)
            Start-Process -FilePath ffmpeg -ArgumentList $ff -WindowStyle Hidden | Out-Null
            Start-Sleep -Milliseconds 900
            Write-Host "recording $out for $($a[1]) s"
        }
        'expect' {
            $pattern = $rest.Trim("'`"")
            $new = @(Get-Content $logFile | Select-Object -Skip $logMark)
            $hit = $new | Where-Object { $_ -match $pattern } | Select-Object -First 1
            if ($hit) { Write-Host "PASS  $pattern   <-  $($hit -replace '^\[[^\]]+\]\[\s*\d+\]', '')" -ForegroundColor Green }
            else { Write-Host "FAIL  $pattern" -ForegroundColor Red; $failed = $true; $new | Where-Object { $_ -match 'LogMJ' } | Select-Object -Last 4 | ForEach-Object { Write-Host "      $($_ -replace '^\[[^\]]+\]\[\s*\d+\]', '')" } }
            $logMark += $new.Count
        }
        default { throw "unknown step '$step'" }
    }
}
if (-not $KeepOpen) { Get-Process MegaJam26* -ErrorAction SilentlyContinue | Stop-Process -Force }
if ($failed) { exit 1 }
