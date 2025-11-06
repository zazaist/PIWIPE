@echo off
echo Windows SDK Installation - Detailed Guide
echo =========================================

echo.
echo Current Windows Version:
systeminfo | findstr "OS Name"

echo.
echo Checking for existing Windows SDK installations...
if exist "C:\Program Files (x86)\Windows Kits\10\Include" (
    echo Windows 10 SDK found at: C:\Program Files (x86)\Windows Kits\10\Include
    goto test_compilation
)

if exist "C:\Program Files\Windows Kits\10\Include" (
    echo Windows 10 SDK found at: C:\Program Files\Windows Kits\10\Include
    goto test_compilation
)

echo Windows SDK not found. Installing...

echo.
echo Method 1: Using Visual Studio Installer
echo ======================================
echo 1. Visual Studio Installer should be open
echo 2. Click "Modify" next to "Visual Studio Build Tools 2022"
echo 3. Go to "Individual components" tab
echo 4. Search for "Windows 10 SDK"
echo 5. Check "Windows 10 SDK (10.0.19041.0)" or latest version
echo 6. Click "Modify" to install
echo.

echo Method 2: Direct Download
echo ========================
echo Download from: https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/
echo Choose: Windows 10 SDK (10.0.19041.0)
echo.

echo Method 3: Using winget (if available)
echo ====================================
winget search "Windows SDK" 2>nul
if %errorlevel% equ 0 (
    echo winget is available. You can install with:
    echo winget install Microsoft.WindowsSDK.10.0.19041
) else (
    echo winget is not available on this system.
)

echo.
echo Method 4: Using chocolatey (if available)
echo ========================================
choco search "windows-sdk" 2>nul
if %errorlevel% equ 0 (
    echo chocolatey is available. You can install with:
    echo choco install windows-sdk-10-version-19041 -y
) else (
    echo chocolatey is not available on this system.
)

echo.
echo Please install Windows SDK using one of the methods above.
echo After installation, run this script again to test compilation.
echo.

:test_compilation
echo Testing compilation...
if exist "C:\Program Files (x86)\Windows Kits\10\Include" (
    echo Windows SDK found! Testing compilation...
    call build-gui.bat
) else (
    echo Windows SDK still not found. Please install it first.
)

pause
