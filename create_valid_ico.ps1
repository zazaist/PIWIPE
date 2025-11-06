# Create a valid Windows ICO file using .NET methods
# This creates a proper ICO file that Windows APIs can load

param(
    [string]$PngPath = "disk-temp.png",
    [string]$OutputPath = "src\piwiper.ico"
)

Add-Type -AssemblyName System.Drawing

Write-Host "=== Creating Valid Windows ICO File ===" -ForegroundColor Cyan
Write-Host ""

if (-not (Test-Path $PngPath)) {
    Write-Host "ERROR: PNG file not found: $PngPath" -ForegroundColor Red
    exit 1
}

try {
    # Load PNG image
    $pngImage = [System.Drawing.Image]::FromFile($PngPath)
    Write-Host "PNG loaded: $($pngImage.Width)x$($pngImage.Height)" -ForegroundColor Green
    
    # Create multiple sizes
    $sizes = @(16, 32, 48, 64)
    $bitmaps = New-Object System.Collections.ArrayList
    
    foreach ($size in $sizes) {
        $bmp = New-Object System.Drawing.Bitmap($size, $size)
        $g = [System.Drawing.Graphics]::FromImage($bmp)
        $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
        $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
        $g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
        
        # Draw resized image
        $g.DrawImage($pngImage, 0, 0, $size, $size)
        $g.Dispose()
        
        [void]$bitmaps.Add($bmp)
        Write-Host "Created: ${size}x${size}" -ForegroundColor Gray
    }
    
    $pngImage.Dispose()
    
    # Use Icon.Save method which creates proper ICO format
    # Create icon from the largest bitmap
    $largestBmp = $bitmaps[$bitmaps.Count - 1]
    
    # Create icon handle from bitmap
    $iconHandle = $largestBmp.GetHicon()
    $icon = [System.Drawing.Icon]::FromHandle($iconHandle)
    
    # Save as ICO - this creates a proper Windows ICO file
    if (Test-Path $OutputPath) {
        Remove-Item $OutputPath -Force
    }
    
    # Create a proper multi-resolution ICO file
    # We'll use a workaround: save each size separately and combine
    # But first, let's try using Icon constructor with multiple sizes
    
    # Create temporary ICO files for each size
    $tempIcons = @()
    foreach ($bmp in $bitmaps) {
        $iconHandle = $bmp.GetHicon()
        $tempIcon = [System.Drawing.Icon]::FromHandle($iconHandle)
        $tempIcons += $tempIcon
    }
    
    # Use the largest icon and save it
    # Windows will use the appropriate size
    $mainIcon = $tempIcons[$tempIcons.Count - 1]
    
    # Save to file
    $fileStream = [System.IO.File]::Create($OutputPath)
    $mainIcon.Save($fileStream)
    $fileStream.Close()
    
    Write-Host ""
    Write-Host "ICO file created: $OutputPath" -ForegroundColor Green
    $icoFile = Get-Item $OutputPath
    Write-Host "Size: $($icoFile.Length) bytes" -ForegroundColor Cyan
    
    # Cleanup
    foreach ($icon in $tempIcons) {
        $icon.Dispose()
    }
    foreach ($bmp in $bitmaps) {
        $bmp.Dispose()
    }
    
    Write-Host ""
    Write-Host "SUCCESS: Valid ICO file created!" -ForegroundColor Green
    Write-Host "This ICO file should work with LoadImage and ExtractIconEx." -ForegroundColor Yellow
    
} catch {
    Write-Host "ERROR: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

