@echo off
echo Creating simple icon for PIWIPER...

REM Create a simple 16x16 icon using PowerShell
powershell -Command "
$bitmap = New-Object System.Drawing.Bitmap(16, 16)
$graphics = [System.Drawing.Graphics]::FromImage($bitmap)
$graphics.Clear([System.Drawing.Color]::DarkBlue)
$brush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::White)
$font = New-Object System.Drawing.Font('Arial', 8, [System.Drawing.FontStyle]::Bold)
$graphics.DrawString('P', $font, $brush, 2, 2)
$graphics.Dispose()
$bitmap.Save('src\piwiper.ico', [System.Drawing.Imaging.ImageFormat]::Icon)
$bitmap.Dispose()
"

if exist src\piwiper.ico (
    echo Icon created successfully: src\piwiper.ico
) else (
    echo Failed to create icon
)

pause