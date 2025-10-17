@echo off
echo ========================================
echo    PIWIPER ICON TEST SCRIPT
echo ========================================
echo.

echo Testing icon integration...
echo.

echo [1/3] Checking icon files...
if exist "src\piwiper_new.ico" (
    echo ✅ piwiper_new.ico found
    dir "src\piwiper_new.ico"
) else (
    echo ❌ piwiper_new.ico not found
)

if exist "src\piwiper.ico" (
    echo ✅ piwiper.ico found
    dir "src\piwiper.ico"
) else (
    echo ❌ piwiper.ico not found
)

echo.
echo [2/3] Building with icon...
call build-gui.bat

echo.
echo [3/3] Testing executable...
if exist "piwiper-gui.exe" (
    echo ✅ piwiper-gui.exe created successfully
    echo.
    echo Icon should now be visible in:
    echo - Window title bar
    echo - Taskbar
    echo - File Explorer
    echo - Task Manager
    echo.
    echo Testing application...
    start piwiper-gui.exe
    echo.
    echo Application started! Check for icon in:
    echo 1. Window title bar (top-left corner)
    echo 2. Taskbar icon
    echo 3. Alt+Tab switcher
    echo.
) else (
    echo ❌ piwiper-gui.exe not created
    echo Build may have failed. Check for errors above.
)

echo.
echo ========================================
echo    ICON TEST COMPLETED
echo ========================================
echo.
pause
