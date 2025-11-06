# Windows SDK Installation Script
Write-Host "Windows SDK Installation Script" -ForegroundColor Green
Write-Host "================================" -ForegroundColor Green

# Check if Windows SDK is already installed
$sdkPath = "C:\Program Files (x86)\Windows Kits\10\Include"
if (Test-Path $sdkPath) {
    Write-Host "Windows SDK already installed at: $sdkPath" -ForegroundColor Green
    Write-Host "Testing compilation..." -ForegroundColor Yellow
    & ".\build-gui.bat"
    exit 0
}

Write-Host "Windows SDK not found. Installing..." -ForegroundColor Yellow

# Method 1: Try winget
try {
    Write-Host "Trying winget installation..." -ForegroundColor Cyan
    winget install Microsoft.WindowsSDK.10.0.19041 --accept-package-agreements --accept-source-agreements
    Write-Host "Windows SDK installed via winget!" -ForegroundColor Green
    exit 0
} catch {
    Write-Host "winget installation failed: $($_.Exception.Message)" -ForegroundColor Yellow
}

# Method 2: Try chocolatey
try {
    Write-Host "Trying chocolatey installation..." -ForegroundColor Cyan
    choco install windows-sdk-10-version-19041 -y
    Write-Host "Windows SDK installed via chocolatey!" -ForegroundColor Green
    exit 0
} catch {
    Write-Host "chocolatey installation failed: $($_.Exception.Message)" -ForegroundColor Yellow
}

# Method 3: Manual installation
Write-Host "Manual installation required:" -ForegroundColor Yellow
Write-Host "1. Download Windows 10 SDK from:" -ForegroundColor Cyan
Write-Host "   https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/" -ForegroundColor Cyan
Write-Host "2. Choose: Windows 10 SDK (10.0.19041.0)" -ForegroundColor Cyan
Write-Host "3. Run the installer as Administrator" -ForegroundColor Cyan
Write-Host "4. Select 'Windows 10 SDK (10.0.19041.0)' and install" -ForegroundColor Cyan

# Open download page
Start-Process "https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/"

Write-Host "Press any key after installation is complete..." -ForegroundColor Green
Read-Host

# Verify installation
if (Test-Path $sdkPath) {
    Write-Host "Windows SDK installation verified!" -ForegroundColor Green
    Write-Host "Testing compilation..." -ForegroundColor Yellow
    & ".\build-gui.bat"
} else {
    Write-Host "Windows SDK installation not found. Please check the installation." -ForegroundColor Red
}






