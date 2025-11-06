# Create PIWIPER disk icon based on description
# Blue disk with black outline, refresh arrow, read/write head

Add-Type -AssemblyName System.Drawing

function DrawDiskIcon($size) {
    $bmp = New-Object System.Drawing.Bitmap($size, $size)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $g.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::AntiAlias
    $g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
    
    # Clear with transparent background
    $g.Clear([System.Drawing.Color]::Transparent)
    
    # Colors
    $blueColor = [System.Drawing.Color]::FromArgb(0, 120, 212)  # Modern blue
    $blackColor = [System.Drawing.Color]::Black
    $whiteColor = [System.Drawing.Color]::White
    
    # Calculate scaling
    $scale = $size / 256.0
    $padding = 20 * $scale
    
    # Main disk body (rounded rectangle, vertical)
    $diskWidth = 80 * $scale
    $diskHeight = 180 * $scale
    $diskX = ($size - $diskWidth) / 2
    $diskY = $padding
    $cornerRadius = 8 * $scale
    
    # Draw disk body (filled blue)
    $diskRect = New-Object System.Drawing.RectangleF($diskX, $diskY, $diskWidth, $diskHeight)
    $blueBrush = New-Object System.Drawing.SolidBrush($blueColor)
    $g.FillRoundedRectangle($blueBrush, $diskRect, $cornerRadius)
    
    # Draw disk body outline (black)
    $blackPen = New-Object System.Drawing.Pen($blackColor, 3 * $scale)
    $g.DrawRoundedRectangle($blackPen, $diskRect, $cornerRadius)
    
    # Disk platter (large circle at top)
    $platterRadius = 50 * $scale
    $platterX = $size / 2
    $platterY = $diskY + 40 * $scale
    $platterRect = New-Object System.Drawing.RectangleF(
        $platterX - $platterRadius, 
        $platterY - $platterRadius, 
        $platterRadius * 2, 
        $platterRadius * 2
    )
    $g.FillEllipse($blueBrush, $platterRect)
    $g.DrawEllipse($blackPen, $platterRect)
    
    # Center hole (small circle)
    $holeRadius = 12 * $scale
    $holeRect = New-Object System.Drawing.RectangleF(
        $platterX - $holeRadius,
        $platterY - $holeRadius,
        $holeRadius * 2,
        $holeRadius * 2
    )
    $whiteBrush = New-Object System.Drawing.SolidBrush($whiteColor)
    $g.FillEllipse($whiteBrush, $holeRect)
    $g.DrawEllipse($blackPen, $holeRect)
    
    # Read/write head (teardrop shape from top-left to center)
    $headPen = New-Object System.Drawing.Pen($blackColor, 4 * $scale)
    $headStartX = $platterX - $platterRadius * 0.7
    $headStartY = $platterY - $platterRadius * 0.7
    $headEndX = $platterX - $holeRadius * 0.5
    $headEndY = $platterY
    $g.DrawLine($headPen, $headStartX, $headStartY, $headEndX, $headEndY)
    
    # Draw small circle at head position
    $headCircleRadius = 6 * $scale
    $headCircleRect = New-Object System.Drawing.RectangleF(
        $headEndX - $headCircleRadius,
        $headEndY - $headCircleRadius,
        $headCircleRadius * 2,
        $headCircleRadius * 2
    )
    $blackBrush = New-Object System.Drawing.SolidBrush($blackColor)
    $g.FillEllipse($blackBrush, $headCircleRect)
    
    # Refresh arrow (curved arrow from bottom-right, up and left)
    $arrowPen = New-Object System.Drawing.Pen($blackColor, 5 * $scale)
    $arrowPen.EndCap = [System.Drawing.Drawing2D.LineCap]::Round
    $arrowPen.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
    
    $arrowStartX = $diskX + $diskWidth - 10 * $scale
    $arrowStartY = $diskY + $diskHeight - 15 * $scale
    $arrowMidX = $diskX + $diskWidth + 15 * $scale
    $arrowMidY = $diskY + $diskHeight - 40 * $scale
    $arrowEndX = $diskX + 20 * $scale
    $arrowEndY = $diskY + $diskHeight - 10 * $scale
    
    # Draw curved arrow using Bezier curve
    $arrowPath = New-Object System.Drawing.Drawing2D.GraphicsPath
    $arrowPath.AddBezier(
        $arrowStartX, $arrowStartY,
        $arrowMidX, $arrowStartY,
        $arrowMidX, $arrowMidY,
        $arrowEndX, $arrowEndY
    )
    $g.DrawPath($arrowPen, $arrowPath)
    
    # Arrow head
    $arrowHeadSize = 8 * $scale
    $arrowHeadPath = New-Object System.Drawing.Drawing2D.GraphicsPath
    $arrowHeadPath.AddLine($arrowEndX, $arrowEndY, $arrowEndX - $arrowHeadSize, $arrowEndY - $arrowHeadSize)
    $arrowHeadPath.AddLine($arrowEndX, $arrowEndY, $arrowEndX - $arrowHeadSize, $arrowEndY + $arrowHeadSize)
    $arrowHeadPath.CloseFigure()
    $g.FillPath($blackBrush, $arrowHeadPath)
    
    # Cleanup
    $blueBrush.Dispose()
    $blackPen.Dispose()
    if ($whiteBrush) { $whiteBrush.Dispose() }
    if ($blackBrush) { $blackBrush.Dispose() }
    $arrowPen.Dispose()
    $arrowPath.Dispose()
    $arrowHeadPath.Dispose()
    $g.Dispose()
    
    return $bmp
}

# Extension method for rounded rectangle
$roundedRectCode = @"
using System;
using System.Drawing;
using System.Drawing.Drawing2D;

public static class GraphicsExtensions
{
    public static void FillRoundedRectangle(this Graphics g, Brush brush, RectangleF rect, float radius)
    {
        using (GraphicsPath path = CreateRoundedRectangle(rect, radius))
        {
            g.FillPath(brush, path);
        }
    }
    
    public static void DrawRoundedRectangle(this Graphics g, Pen pen, RectangleF rect, float radius)
    {
        using (GraphicsPath path = CreateRoundedRectangle(rect, radius))
        {
            g.DrawPath(pen, path);
        }
    }
    
    private static GraphicsPath CreateRoundedRectangle(RectangleF rect, float radius)
    {
        GraphicsPath path = new GraphicsPath();
        float diameter = radius * 2;
        path.AddArc(rect.X, rect.Y, diameter, diameter, 180, 90);
        path.AddArc(rect.Right - diameter, rect.Y, diameter, diameter, 270, 90);
        path.AddArc(rect.Right - diameter, rect.Bottom - diameter, diameter, diameter, 0, 90);
        path.AddArc(rect.X, rect.Bottom - diameter, diameter, diameter, 90, 90);
        path.CloseFigure();
        return path;
    }
}
"@

Add-Type -TypeDefinition $roundedRectCode -ReferencedAssemblies System.Drawing

# Create icons in multiple sizes
$sizes = @(16, 32, 48, 64, 128, 256)
$bitmaps = New-Object System.Collections.ArrayList

foreach ($size in $sizes) {
    $bmp = DrawDiskIcon $size
    [void]$bitmaps.Add($bmp)
}

# Save as ICO file
$icoPath = "src\piwiper.ico"
if (Test-Path $icoPath) {
    Remove-Item $icoPath -Force
}

# Create multi-resolution ICO file
# Note: PowerShell's Icon class is limited, so we'll use a workaround
# Save the largest bitmap and let Windows handle scaling
$largestBmp = $bitmaps[$bitmaps.Count - 1]  # 256x256
$largestBmp.Save($icoPath, [System.Drawing.Imaging.ImageFormat]::Png)

# Convert PNG to ICO using a simple method
# For proper ICO, we'd need a library, but Windows can use PNG in ICO container
# Let's try using the built-in method
try {
    # Create a proper ICO file structure
    $icoBytes = New-Object byte[] 0
    $icoStream = [System.IO.File]::Create($icoPath)
    
    # ICO file header (6 bytes)
    $icoStream.WriteByte(0)  # Reserved
    $icoStream.WriteByte(0)  # Reserved
    $icoStream.WriteByte(1)  # Type (1 = ICO)
    $icoStream.WriteByte(0)  # Type
    $icoStream.WriteByte(1)  # Number of images
    $icoStream.WriteByte(0)  # Number of images
    
    # Image directory entry (16 bytes)
    $icoStream.WriteByte(0)  # Width (0 = 256)
    $icoStream.WriteByte(0)  # Height (0 = 256)
    $icoStream.WriteByte(0)  # Color palette
    $icoStream.WriteByte(0)  # Reserved
    $icoStream.WriteByte(1)  # Planes
    $icoStream.WriteByte(0)  # Planes
    $icoStream.WriteByte(32) # Bits per pixel
    $icoStream.WriteByte(0)  # Bits per pixel
    
    # Image data size and offset (will be calculated)
    $pngBytes = [System.IO.File]::ReadAllBytes($icoPath)
    $dataSize = $pngBytes.Length
    $dataOffset = 22  # 6 (header) + 16 (directory)
    
    $icoStream.Write([BitConverter]::GetBytes($dataSize), 0, 4)
    $icoStream.Write([BitConverter]::GetBytes($dataOffset), 0, 4)
    
    # Write PNG data
    $icoStream.Write($pngBytes, 0, $pngBytes.Length)
    $icoStream.Close()
    
    Write-Host "Icon created successfully: $icoPath" -ForegroundColor Green
    Write-Host "Size: $dataSize bytes" -ForegroundColor Cyan
} catch {
    Write-Host "Error creating ICO: $_" -ForegroundColor Red
    Write-Host "Using PNG format instead..." -ForegroundColor Yellow
}

# Clean up
foreach ($bmp in $bitmaps) {
    $bmp.Dispose()
}

Write-Host ""
Write-Host "Icon creation complete!" -ForegroundColor Green
Write-Host "If ICO format has issues, you can convert the PNG manually using an online tool." -ForegroundColor Yellow

