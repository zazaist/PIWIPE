@echo off
echo Creating simple disk icon...

REM Create a basic ICO file using PowerShell
powershell -Command "Add-Type -AssemblyName System.Drawing; $bitmap = New-Object System.Drawing.Bitmap(32, 32); $graphics = [System.Drawing.Graphics]::FromImage($bitmap); $graphics.Clear([System.Drawing.Color]::FromArgb(0, 120, 212)); $brush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::White); $graphics.FillEllipse($brush, 4, 4, 24, 24); $pen = New-Object System.Drawing.Pen([System.Drawing.Color]::White, 2); $graphics.DrawEllipse($pen, 4, 4, 24, 24); $graphics.FillEllipse($brush, 12, 12, 8, 8); $font = New-Object System.Drawing.Font('Arial', 8, [System.Drawing.FontStyle]::Bold); $textBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::White); $graphics.DrawString('P', $font, $textBrush, 10, 8); $graphics.Dispose(); $brush.Dispose(); $pen.Dispose(); $font.Dispose(); $textBrush.Dispose(); $bitmap.Save('src\piwiper.ico', [System.Drawing.Imaging.ImageFormat]::Icon); $bitmap.Dispose()"

if exist "src\piwiper.ico" (
    echo ✅ Simple icon created!
    echo Now recompiling...
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    cl /EHsc /W4 /std:c++17 src\main_gui.cpp /Fe:piwiper-gui.exe user32.lib comctl32.lib gdi32.lib msimg32.lib advapi32.lib shell32.lib /link /SUBSYSTEM:WINDOWS
    if %ERRORLEVEL% equ 0 (
        mt.exe -manifest src\piwiper-gui.manifest -outputresource:piwiper-gui.exe;1
        echo ✅ Recompiled with new icon!
    )
) else (
    echo ❌ Failed to create icon
)

pause
