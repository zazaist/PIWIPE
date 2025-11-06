@echo off
echo Windows SDK Check
echo =================

echo Checking for Windows SDK...

if exist "C:\Program Files (x86)\Windows Kits\10\Include" (
    echo Windows 10 SDK found!
    echo Path: C:\Program Files (x86)\Windows Kits\10\Include
    goto :found
)

if exist "C:\Program Files\Windows Kits\10\Include" (
    echo Windows 10 SDK found!
    echo Path: C:\Program Files\Windows Kits\10\Include
    goto :found
)

echo Windows SDK not found!
echo.
echo Please install Windows 10 SDK from:
echo https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/
echo.
echo Or use Visual Studio Installer:
echo 1. Open Visual Studio Installer
echo 2. Modify Visual Studio Build Tools 2022
echo 3. Individual components tab
echo 4. Search for "Windows 10 SDK"
echo 5. Install Windows 10 SDK (10.0.19041.0)
echo.
goto :end

:found
echo.
echo Testing compilation...
call build-gui.bat

:end
pause






