<#
.SYNOPSIS
    Save a PNG of a window (by process name or id) as it appears on screen. Used for dev-log screenshots of the packaged game.
.EXAMPLE
    .\tools\capture-window.ps1 -ProcessName MegaJam26 -Out devlog\img\shot.png
#>
[CmdletBinding()]
param(
    [string]$ProcessName = 'MegaJam26',
    [int]$ProcessId = 0,
    [Parameter(Mandatory = $true)][string]$Out,
    [switch]$Foreground
)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class MJWin {
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
    [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X, Y; }
    [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr hWnd, out RECT rect);
    [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr hWnd, ref POINT pt);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int cmd);
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
}
'@
[void][MJWin]::SetProcessDPIAware()
$proc = if ($ProcessId) { Get-Process -Id $ProcessId } else { Get-Process -Name $ProcessName | Where-Object { $_.MainWindowHandle -ne 0 } | Select-Object -First 1 }
if (-not $proc -or $proc.MainWindowHandle -eq 0) { throw "No window found for '$ProcessName' $ProcessId" }
$h = $proc.MainWindowHandle
if ($Foreground) { [void][MJWin]::ShowWindow($h, 9); [void][MJWin]::SetForegroundWindow($h); Start-Sleep -Milliseconds 400 }
$rect = New-Object MJWin+RECT
[void][MJWin]::GetClientRect($h, [ref]$rect)
$pt = New-Object MJWin+POINT
[void][MJWin]::ClientToScreen($h, [ref]$pt)
$w = $rect.Right - $rect.Left; $hgt = $rect.Bottom - $rect.Top
if ($w -le 0 -or $hgt -le 0) { throw 'Window has no client area (minimised?)' }
$bmp = New-Object System.Drawing.Bitmap $w, $hgt
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($pt.X, $pt.Y, 0, 0, $bmp.Size)
$full = [System.IO.Path]::GetFullPath((Join-Path (Get-Location) $Out))
New-Item -ItemType Directory -Force -Path (Split-Path $full) | Out-Null
$bmp.Save($full, [System.Drawing.Imaging.ImageFormat]::Png)
$g.Dispose(); $bmp.Dispose()
Write-Host "$full ($w x $hgt)"
