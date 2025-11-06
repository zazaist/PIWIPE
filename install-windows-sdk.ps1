# Windows SDK Installation Script
Write-Host "Windows SDK Installation Script" -ForegroundColor Green
Write-Host "================================" -ForegroundColor Green

# Check if running as administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")

if (-not $isAdmin) {
    Write-Host "This script requires administrator privileges. Please run PowerShell as Administrator." -ForegroundColor Red
    Write-Host "Right-click on PowerShell and select 'Run as Administrator'" -ForegroundColor Yellow
    pause
    exit 1
}

Write-Host "Installing Windows 10 SDK..." -ForegroundColor Yellow

# Method 1: Try to install via winget (if available)
try {
    Write-Host "Trying to install via winget..." -ForegroundColor Cyan
    winget install Microsoft.WindowsSDK.10.0.19041
    Write-Host "Windows SDK installed successfully via winget!" -ForegroundColor Green
    exit 0
} catch {
    Write-Host "winget installation failed, trying alternative method..." -ForegroundColor Yellow
}

# Method 2: Try to install via chocolatey (if available)
try {
    Write-Host "Trying to install via chocolatey..." -ForegroundColor Cyan
    choco install windows-sdk-10-version-19041 -y
    Write-Host "Windows SDK installed successfully via chocolatey!" -ForegroundColor Green
    exit 0
} catch {
    Write-Host "chocolatey installation failed, trying manual download..." -ForegroundColor Yellow
}

# Method 3: Manual download and install
Write-Host "Please download and install Windows 10 SDK manually:" -ForegroundColor Yellow
Write-Host "1. Go to: https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/" -ForegroundColor Cyan
Write-Host "2. Download Windows 10 SDK (10.0.19041.0)" -ForegroundColor Cyan
Write-Host "3. Run the installer as Administrator" -ForegroundColor Cyan
Write-Host "4. Select 'Windows 10 SDK (10.0.19041.0)' and install" -ForegroundColor Cyan

# Open the download page
Start-Process "https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/"

Write-Host "Press any key after installation is complete..." -ForegroundColor Green
pause

# Verify installation
if (Test-Path "C:\Program Files (x86)\Windows Kits\10\Include") {
    Write-Host "Windows SDK installation verified!" -ForegroundColor Green
} else {
    Write-Host "Windows SDK installation not found. Please check the installation." -ForegroundColor Red
}






