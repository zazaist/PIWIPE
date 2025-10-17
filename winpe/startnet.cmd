@echo off
wpeinit
color 0A

echo ===============================
echo   PIWIPER Environment
echo ===============================

echo Launching PIWIPER...
if exist X:\piwiper.exe (
    X:\piwiper.exe
) else (
    echo GUI fallback...
    powershell -ExecutionPolicy Bypass -NoLogo -NoProfile -File X:\wipe-gui.ps1
)

echo.
echo All done. You may now power off this machine.
cmd /k
