# Test icon loading with debug output
Write-Host "=== PIWIPER Icon Debug Test ===" -ForegroundColor Cyan
Write-Host ""

# Check if icon file exists
if (Test-Path "src\piwiper.ico") {
    $ico = Get-Item "src\piwiper.ico"
    Write-Host "Icon file found: src\piwiper.ico" -ForegroundColor Green
    Write-Host "  Size: $($ico.Length) bytes" -ForegroundColor Gray
    Write-Host "  Modified: $($ico.LastWriteTime)" -ForegroundColor Gray
} else {
    Write-Host "WARNING: Icon file not found: src\piwiper.ico" -ForegroundColor Red
}

Write-Host ""
Write-Host "Starting application and monitoring debug output..." -ForegroundColor Yellow
Write-Host "Debug messages will appear below:" -ForegroundColor Yellow
Write-Host ""

# Start the application
$proc = Start-Process -FilePath ".\piwiper-gui-debug.exe" -Verb RunAs -PassThru -WindowStyle Normal

# Wait a bit for the app to start
Start-Sleep -Seconds 2

# Try to capture debug output using DbgView or just show instructions
Write-Host "To view debug output:" -ForegroundColor Cyan
Write-Host "  1. Download DebugView from Microsoft Sysinternals" -ForegroundColor White
Write-Host "  2. Run DebugView as Administrator" -ForegroundColor White
Write-Host "  3. Filter for '[ICON]' messages" -ForegroundColor White
Write-Host ""
Write-Host "Or check the output in Visual Studio Output window if debugging." -ForegroundColor White
Write-Host ""
Write-Host "Application started with PID: $($proc.Id)" -ForegroundColor Green
Write-Host "Check the application window for the icon." -ForegroundColor Yellow

