# End-to-end mouse/keyboard test for VocabCross on Windows.
# 1) Crossword: open BRIBERY directly, type the target word, reveal the rest.
# 2) Image guess: open ECLIPSE directly, click its seven keys.
# 3) Map: click the first island node to open the real wheel crossword.
#
# Usage: powershell -ExecutionPolicy Bypass -File tools/interactive_test.ps1

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

$src = @'
using System;
using System.Runtime.InteropServices;
public class VCWin32 {
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int X, int Y);
  [DllImport("user32.dll")] public static extern void mouse_event(uint dwFlags, uint dx, uint dy, uint dwData, UIntPtr dwExtraInfo);
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr hWnd, out RECT lpRect);
  [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr hWnd, ref POINT lpPoint);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
  [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X, Y; }
}
'@
Add-Type -TypeDefinition $src

function StartGame([string[]]$gameArgs) {
    $exe = Join-Path $root 'VocabCross.exe'
    if ($gameArgs -and $gameArgs.Count -gt 0) {
        $p = Start-Process -FilePath $exe -ArgumentList $gameArgs -PassThru
    } else {
        $p = Start-Process -FilePath $exe -PassThru
    }
    Start-Sleep -Seconds 2
    $p.Refresh()
    return $p
}

function Geometry($p) {
    $h = $p.MainWindowHandle
    [VCWin32]::SetForegroundWindow($h) | Out-Null
    Start-Sleep -Milliseconds 300
    $r = New-Object VCWin32+RECT
    [VCWin32]::GetClientRect($h, [ref]$r) | Out-Null
    $cw = $r.Right - $r.Left
    $ch = $r.Bottom - $r.Top
    $o = New-Object VCWin32+POINT
    $o.X = 0; $o.Y = 0
    [VCWin32]::ClientToScreen($h, [ref]$o) | Out-Null
    $scale = [Math]::Min($cw / 540.0, $ch / 960.0)
    $dx = ($cw - 540 * $scale) / 2
    $dy = ($ch - 960 * $scale) / 2
    return @{ W = $cw; H = $ch; X = $o.X; Y = $o.Y; S = $scale; DX = $dx; DY = $dy; HWND = $h }
}

function ClickCanvas($g, [double]$cx, [double]$cy) {
    $sx = [int]($g.X + $g.DX + $cx * $g.S)
    $sy = [int]($g.Y + $g.DY + $cy * $g.S)
    [VCWin32]::SetCursorPos($sx, $sy) | Out-Null
    Start-Sleep -Milliseconds 100
    [VCWin32]::mouse_event(0x0002, 0, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 60
    [VCWin32]::mouse_event(0x0004, 0, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 300
}

function Capture($g, [string]$name) {
    Add-Type -AssemblyName System.Drawing
    $bmp = New-Object System.Drawing.Bitmap $g.W, $g.H
    $gfx = [System.Drawing.Graphics]::FromImage($bmp)
    $gfx.CopyFromScreen($g.X, $g.Y, 0, 0, $bmp.Size)
    $bmp.Save((Join-Path $root $name))
    $gfx.Dispose(); $bmp.Dispose()
}

function MoveCanvas($g, [double]$cx, [double]$cy) {
    $sx = [int]($g.X + $g.DX + $cx * $g.S)
    $sy = [int]($g.Y + $g.DY + $cy * $g.S)
    [VCWin32]::SetCursorPos($sx, $sy) | Out-Null
}

# --- 1. Crossword: type the target word, then reveal the rest. -------------
$p = StartGame @('--word', 'BRIBERY')
$g = Geometry $p
Add-Type -AssemblyName System.Windows.Forms
[System.Windows.Forms.SendKeys]::SendWait('BRIBERY')
Start-Sleep -Milliseconds 800
Capture $g 'verification_reports/interactive_crossword_typed.png'
ClickCanvas $g 456 796          # REVEAL
Start-Sleep -Milliseconds 900
Capture $g 'verification_reports/interactive_crossword_reveal.png'
Stop-Process -Id $p.Id
Start-Sleep -Milliseconds 400

# --- 2. Image guess: click ECLIPSE on the 2x7 keyboard. --------------------
$p = StartGame @('--word', 'ECLIPSE')
$g = Geometry $p
Capture $g 'verification_reports/interactive_image_stage.png'
ClickCanvas $g 60 760     # E
ClickCanvas $g 60 834     # C
ClickCanvas $g 490 760    # L
ClickCanvas $g 340 760    # I
ClickCanvas $g 130 760    # P
ClickCanvas $g 200 760    # S
ClickCanvas $g 270 834    # E (second)
Start-Sleep -Milliseconds 900
Capture $g 'verification_reports/interactive_image_victory.png'
Stop-Process -Id $p.Id
Start-Sleep -Milliseconds 400

# --- 3. Map -> first node -> real wheel crossword. -------------------------
$p = StartGame @()
$g = Geometry $p
ClickCanvas $g 270 768   # node 1
Start-Sleep -Milliseconds 900
Capture $g 'verification_reports/interactive_wheel_stage.png'

# Three-letter drag: the line must stay connected even when the path is not
# yet a valid prefix. It turns red, but it must not reset mid-drag.
MoveCanvas $g 270 617
Start-Sleep -Milliseconds 150
[VCWin32]::mouse_event(0x0002, 0, 0, 0, [UIntPtr]::Zero)
Start-Sleep -Milliseconds 200
MoveCanvas $g 388 735
Start-Sleep -Milliseconds 250
MoveCanvas $g 270 853
Start-Sleep -Milliseconds 300
Capture $g 'verification_reports/interactive_wheel_drag3.png'
[VCWin32]::mouse_event(0x0004, 0, 0, 0, [UIntPtr]::Zero)
Start-Sleep -Milliseconds 800
Capture $g 'verification_reports/interactive_wheel_after_release.png'
Stop-Process -Id $p.Id

Write-Host 'Interactive tests finished. Captured crossword, image, wheel stage and 3-letter drag.'
