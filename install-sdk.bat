@echo off
echo Windows SDK Installation Helper
echo ===============================

echo.
echo Please follow these steps to install Windows SDK:
echo.
echo 1. Visual Studio Installer should be open now
echo 2. Click on "Modify" next to "Visual Studio Build Tools 2022"
echo 3. Go to "Individual components" tab
echo 4. Search for "Windows 10 SDK"
echo 5. Check "Windows 10 SDK (10.0.19041.0)" or latest version
echo 6. Click "Modify" to install
echo.
echo Alternative: Download directly from Microsoft:
echo https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/
echo.

pause

echo.
echo Checking if Windows SDK is installed...
if exist "C:\Program Files (x86)\Windows Kits\10\Include" (
    echo Windows SDK found! Installation successful.
) else (
    echo Windows SDK not found. Please install it manually.
    echo.
    echo Opening Microsoft download page...
    start https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/
)

echo.
echo After installation, run build-gui.bat to test compilation.
pause






