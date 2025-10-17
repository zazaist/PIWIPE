@echo off
echo Building piwiper GUI with MSVC (simple)...

REM Check if Visual Studio is available
where cl >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Visual Studio not found. Please run from "Developer Command Prompt for VS"
    pause
    exit /b 1
)

echo Compiling...
cl /EHsc /W4 /std:c++17 /utf-8 /I. src\main_gui.cpp /Fe:piwiper-gui-test.exe user32.lib comctl32.lib gdi32.lib advapi32.lib shell32.lib /link /SUBSYSTEM:WINDOWS

if %ERRORLEVEL% equ 0 (
    echo Build successful! Executable: piwiper-gui-test.exe
    echo.
    echo NOTE: Run as Administrator!
) else (
    echo Build failed!
)

pause

