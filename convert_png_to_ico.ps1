# PNG to ICO Converter for Windows Resource Compiler
# Creates ICO file compatible with Windows Resource Compiler (old 3.00 format)

param(
    [string]$PngPath = "",
    [string]$OutputPath = "src\piwiper.ico"
)

Add-Type -AssemblyName System.Drawing

function Convert-PngToIco {
    param(
        [string]$PngPath,
        [string]$IcoPath
    )
    
    Write-Host "Converting PNG to ICO..." -ForegroundColor Cyan
    Write-Host "  Input: $PngPath" -ForegroundColor Gray
    Write-Host "  Output: $IcoPath" -ForegroundColor Gray
    
    if (-not (Test-Path $PngPath)) {
        Write-Host "ERROR: PNG file not found: $PngPath" -ForegroundColor Red
        return $false
    }
    
    try {
        # Load PNG image
        $pngImage = [System.Drawing.Image]::FromFile($PngPath)
        Write-Host "  PNG loaded: $($pngImage.Width)x$($pngImage.Height)" -ForegroundColor Green
        
        # Required sizes for ICO (Windows Resource Compiler compatible)
        $sizes = @(16, 32, 48, 64)
        $bitmaps = New-Object System.Collections.ArrayList
        
        foreach ($size in $sizes) {
            # Create bitmap with specific size
            $bmp = New-Object System.Drawing.Bitmap($size, $size)
            $g = [System.Drawing.Graphics]::FromImage($bmp)
            $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
            $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
            $g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
            
            # Draw resized image
            $g.DrawImage($pngImage, 0, 0, $size, $size)
            $g.Dispose()
            
            [void]$bitmaps.Add($bmp)
            Write-Host "  Created: ${size}x${size}" -ForegroundColor Gray
        }
        
        $pngImage.Dispose()
        
        # Create ICO file
        # ICO file format: Header + Directory + Image Data
        $icoStream = [System.IO.File]::Create($IcoPath)
        
        # ICO Header (6 bytes)
        $icoStream.WriteByte(0)  # Reserved (must be 0)
        $icoStream.WriteByte(0)  # Reserved (must be 0)
        $icoStream.WriteByte(1)  # Type (1 = ICO)
        $icoStream.WriteByte(0)  # Type
        $icoStream.WriteByte([byte]$bitmaps.Count)  # Number of images
        $icoStream.WriteByte(0)  # Number of images (high byte)
        
        # Calculate offsets
        $currentOffset = 6 + ($bitmaps.Count * 16)  # Header + Directory entries
        
        # Directory entries (16 bytes each)
        for ($i = 0; $i -lt $bitmaps.Count; $i++) {
            $bmp = $bitmaps[$i]
            $size = $bmp.Width
            
            # Save bitmap to memory stream to get size
            $memStream = New-Object System.IO.MemoryStream
            $bmp.Save($memStream, [System.Drawing.Imaging.ImageFormat]::Bmp)
            $bmpData = $memStream.ToArray()
            $memStream.Dispose()
            
            # Directory entry
            $icoStream.WriteByte([byte]$size)  # Width (0 = 256)
            $icoStream.WriteByte([byte]$size)  # Height (0 = 256)
            $icoStream.WriteByte(0)  # Color palette (0 = no palette)
            $icoStream.WriteByte(0)  # Reserved
            $icoStream.WriteByte(1)  # Planes (must be 1)
            $icoStream.WriteByte(0)  # Planes (high byte)
            $icoStream.WriteByte(32) # Bits per pixel
            $icoStream.WriteByte(0)  # Bits per pixel (high byte)
            
            # Image data size (4 bytes, little-endian)
            $dataSize = $bmpData.Length
            $icoStream.Write([BitConverter]::GetBytes($dataSize), 0, 4)
            
            # Image data offset (4 bytes, little-endian)
            $icoStream.Write([BitConverter]::GetBytes($currentOffset), 0, 4)
            
            $currentOffset += $dataSize
        }
        
        # Write image data
        for ($i = 0; $i -lt $bitmaps.Count; $i++) {
            $bmp = $bitmaps[$i]
            
            # Save bitmap as BMP
            $memStream = New-Object System.IO.MemoryStream
            $bmp.Save($memStream, [System.Drawing.Imaging.ImageFormat]::Bmp)
            $bmpData = $memStream.ToArray()
            $memStream.Dispose()
            
            # Write BMP data
            $icoStream.Write($bmpData, 0, $bmpData.Length)
        }
        
        $icoStream.Close()
        
        # Cleanup
        foreach ($bmp in $bitmaps) {
            $bmp.Dispose()
        }
        
        Write-Host "  ICO created successfully!" -ForegroundColor Green
        $icoFile = Get-Item $IcoPath
        Write-Host "  Size: $($icoFile.Length) bytes" -ForegroundColor Cyan
        
        return $true
    }
    catch {
        Write-Host "ERROR: $($_.Exception.Message)" -ForegroundColor Red
        return $false
    }
}

# Main
Write-Host "=== PNG to ICO Converter ===" -ForegroundColor Cyan
Write-Host ""

if ($PngPath -eq "") {
    # Look for PNG files
    $pngFiles = Get-ChildItem -Path "." -Filter "*.png" -Recurse -ErrorAction SilentlyContinue | Where-Object { $_.FullName -notlike "*\backups\*" }
    
    if ($pngFiles.Count -eq 0) {
        Write-Host "No PNG files found in current directory." -ForegroundColor Yellow
        Write-Host ""
        Write-Host "Usage:" -ForegroundColor Cyan
        Write-Host "  .\convert_png_to_ico.ps1 -PngPath 'path\to\image.png'" -ForegroundColor White
        Write-Host "  .\convert_png_to_ico.ps1 -PngPath 'path\to\image.png' -OutputPath 'src\piwiper.ico'" -ForegroundColor White
        Write-Host ""
        Write-Host "Or place a PNG file in the project root and run this script." -ForegroundColor Yellow
        exit 1
    }
    
    if ($pngFiles.Count -eq 1) {
        $PngPath = $pngFiles[0].FullName
        Write-Host "Found PNG file: $PngPath" -ForegroundColor Green
    } else {
        Write-Host "Multiple PNG files found:" -ForegroundColor Yellow
        for ($i = 0; $i -lt $pngFiles.Count; $i++) {
            Write-Host "  [$i] $($pngFiles[$i].FullName)" -ForegroundColor Gray
        }
        Write-Host ""
        $selection = Read-Host "Select PNG file (0-$($pngFiles.Count-1))"
        $PngPath = $pngFiles[[int]$selection].FullName
    }
}

if (Convert-PngToIco -PngPath $PngPath -IcoPath $OutputPath) {
    Write-Host ""
    Write-Host "SUCCESS: ICO file created at $OutputPath" -ForegroundColor Green
    Write-Host ""
    Write-Host "Next steps:" -ForegroundColor Cyan
    Write-Host "  1. Enable icon in src/piwiper.rc and src/version.rc" -ForegroundColor White
    Write-Host "  2. Run build-gui.bat to compile" -ForegroundColor White
} else {
    Write-Host ""
    Write-Host "FAILED: Could not create ICO file" -ForegroundColor Red
    exit 1
}

