# Create a simple ICO file for PIWIPER
Add-Type -AssemblyName System.Drawing

# Create multiple sizes for the icon
$sizes = @(16, 32, 48, 64, 128, 256)
$bitmaps = New-Object System.Collections.ArrayList

foreach ($size in $sizes) {
    $bmp = New-Object System.Drawing.Bitmap($size, $size)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $g.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::AntiAlias
    
    # Background: Modern blue
    $g.Clear([System.Drawing.Color]::FromArgb(0, 120, 212))
    
    # Draw "P" letter in white
    $brush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::White)
    $fontSize = [int]($size * 0.6)
    $font = New-Object System.Drawing.Font('Arial', $fontSize, [System.Drawing.FontStyle]::Bold)
    
    $rect = New-Object System.Drawing.RectangleF(0, 0, $size, $size)
    $sf = New-Object System.Drawing.StringFormat
    $sf.Alignment = [System.Drawing.StringAlignment]::Center
    $sf.LineAlignment = [System.Drawing.StringAlignment]::Center
    
    $g.DrawString('P', $font, $brush, $rect, $sf)
    
    $g.Dispose()
    [void]$bitmaps.Add($bmp)
}

# Save as ICO file
$icoPath = "src\piwiper.ico"
if (Test-Path $icoPath) {
    Remove-Item $icoPath -Force
}

# Create ICO file using the first bitmap (16x16) as base
$icon = [System.Drawing.Icon]::FromHandle((New-Object System.Drawing.Icon($bitmaps[0], 16, 16)).Handle)
$stream = [System.IO.File]::Create($icoPath)
$icon.Save($stream)
$stream.Close()
$icon.Dispose()

# Clean up
foreach ($bmp in $bitmaps) {
    $bmp.Dispose()
}

Write-Host "Icon created successfully: $icoPath" -ForegroundColor Green

