# PNG to ICO Converter v2 - Windows Resource Compiler Compatible
# Creates ICO with old BMP format compatible with Windows Resource Compiler

param(
    [string]$PngPath = "disk-temp.png",
    [string]$OutputPath = "src\piwiper.ico"
)

Add-Type -AssemblyName System.Drawing

function Create-OldFormatBMP {
    param(
        [System.Drawing.Bitmap]$Bitmap
    )
    
    # Create old-format BMP (BITMAPINFOHEADER, not BITMAPV4HEADER)
    $width = $Bitmap.Width
    $height = $Bitmap.Height
    
    # Calculate row size (must be multiple of 4)
    $rowSize = [Math]::Ceiling(($width * 3) / 4) * 4
    $imageSize = $rowSize * $height
    
    # BMP Header (14 bytes)
    $bmpHeader = New-Object byte[] 14
    $bmpHeader[0] = 0x42  # 'B'
    $bmpHeader[1] = 0x4D   # 'M'
    [BitConverter]::GetBytes([int32](14 + 40 + $imageSize)).CopyTo($bmpHeader, 2)  # File size
    [BitConverter]::GetBytes([int32]0).CopyTo($bmpHeader, 6)  # Reserved
    [BitConverter]::GetBytes([int32]54).CopyTo($bmpHeader, 10)  # Offset to pixel data
    
    # BITMAPINFOHEADER (40 bytes) - Old format
    $infoHeader = New-Object byte[] 40
    [BitConverter]::GetBytes([int32]40).CopyTo($infoHeader, 0)  # Header size
    [BitConverter]::GetBytes([int32]$width).CopyTo($infoHeader, 4)  # Width
    [BitConverter]::GetBytes([int32]($height * 2)).CopyTo($infoHeader, 8)  # Height (doubled for top-down)
    [BitConverter]::GetBytes([int16]1).CopyTo($infoHeader, 12)  # Planes
    [BitConverter]::GetBytes([int16]24).CopyTo($infoHeader, 14)  # Bits per pixel
    [BitConverter]::GetBytes([int32]0).CopyTo($infoHeader, 16)  # Compression (0 = none)
    [BitConverter]::GetBytes([int32]$imageSize).CopyTo($infoHeader, 20)  # Image size
    [BitConverter]::GetBytes([int32]0).CopyTo($infoHeader, 24)  # X pixels per meter
    [BitConverter]::GetBytes([int32]0).CopyTo($infoHeader, 28)  # Y pixels per meter
    [BitConverter]::GetBytes([int32]0).CopyTo($infoHeader, 32)  # Colors used
    [BitConverter]::GetBytes([int32]0).CopyTo($infoHeader, 36)  # Important colors
    
    # Get pixel data (BGR format, bottom-up)
    $pixelData = New-Object byte[] $imageSize
    $index = 0
    
    for ($y = $height - 1; $y -ge 0; $y--) {
        for ($x = 0; $x -lt $width; $x++) {
            $color = $Bitmap.GetPixel($x, $y)
            $pixelData[$index++] = $color.B
            $pixelData[$index++] = $color.G
            $pixelData[$index++] = $color.R
        }
        # Pad row to multiple of 4
        while ($index % 4 -ne 0) {
            $pixelData[$index++] = 0
        }
    }
    
    # Combine all parts
    $bmpData = New-Object byte[] (14 + 40 + $imageSize)
    $bmpHeader.CopyTo($bmpData, 0)
    $infoHeader.CopyTo($bmpData, 14)
    $pixelData.CopyTo($bmpData, 54)
    
    return $bmpData
}

function Convert-PngToIco {
    param(
        [string]$PngPath,
        [string]$IcoPath
    )
    
    Write-Host "Converting PNG to ICO (Windows Resource Compiler compatible)..." -ForegroundColor Cyan
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
        
        # Required sizes for ICO
        $sizes = @(16, 32, 48, 64)
        $bitmaps = New-Object System.Collections.ArrayList
        $bmpDataList = New-Object System.Collections.ArrayList
        
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
            
            # Convert to old-format BMP
            $bmpData = Create-OldFormatBMP -Bitmap $bmp
            [void]$bmpDataList.Add($bmpData)
            
            Write-Host "  Created: ${size}x${size} (BMP: $($bmpData.Length) bytes)" -ForegroundColor Gray
        }
        
        $pngImage.Dispose()
        
        # Create ICO file
        $icoStream = [System.IO.File]::Create($IcoPath)
        
        # ICO Header (6 bytes)
        $icoStream.WriteByte(0)  # Reserved
        $icoStream.WriteByte(0)  # Reserved
        $icoStream.WriteByte(1)  # Type (1 = ICO)
        $icoStream.WriteByte(0)  # Type
        $icoStream.WriteByte([byte]$bitmaps.Count)  # Number of images
        $icoStream.WriteByte(0)  # Number of images
        
        # Calculate offsets
        $currentOffset = 6 + ($bitmaps.Count * 16)
        
        # Directory entries (16 bytes each)
        for ($i = 0; $i -lt $bitmaps.Count; $i++) {
            $bmp = $bitmaps[$i]
            $size = $bmp.Width
            $bmpData = $bmpDataList[$i]
            
            # Directory entry
            $icoStream.WriteByte([byte]$size)  # Width
            $icoStream.WriteByte([byte]$size)  # Height
            $icoStream.WriteByte(0)  # Color palette
            $icoStream.WriteByte(0)  # Reserved
            $icoStream.WriteByte(1)  # Planes
            $icoStream.WriteByte(0)  # Planes
            $icoStream.WriteByte(24) # Bits per pixel (24-bit RGB)
            $icoStream.WriteByte(0)  # Bits per pixel
            
            # Image data size (4 bytes, little-endian)
            $dataSize = $bmpData.Length
            $icoStream.Write([BitConverter]::GetBytes($dataSize), 0, 4)
            
            # Image data offset (4 bytes, little-endian)
            $icoStream.Write([BitConverter]::GetBytes($currentOffset), 0, 4)
            
            $currentOffset += $dataSize
        }
        
        # Write image data (BMP format)
        for ($i = 0; $i -lt $bmpDataList.Count; $i++) {
            $bmpData = $bmpDataList[$i]
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
        Write-Host "Stack: $($_.ScriptStackTrace)" -ForegroundColor Red
        return $false
    }
}

# Main
Write-Host "=== PNG to ICO Converter v2 ===" -ForegroundColor Cyan
Write-Host ""

if (Convert-PngToIco -PngPath $PngPath -IcoPath $OutputPath) {
    Write-Host ""
    Write-Host "SUCCESS: ICO file created at $OutputPath" -ForegroundColor Green
} else {
    Write-Host ""
    Write-Host "FAILED: Could not create ICO file" -ForegroundColor Red
    exit 1
}

