@echo off
echo Building piwiper GUI with MSVC...

REM Check if Visual Studio is available
where cl >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Visual Studio not found. Please run from "Developer Command Prompt for VS"
    echo Or install Visual Studio Build Tools
    pause
    exit /b 1
)

echo Compiling GUI version...
cl /EHsc /W4 /std:c++17 /I. src\main_gui.cpp src\version.rc /Fe:piwiper-gui.exe user32.lib comctl32.lib gdi32.lib msimg32.lib advapi32.lib shell32.lib /link /SUBSYSTEM:WINDOWS

if %ERRORLEVEL% equ 0 (
    echo Embedding Administrator manifest...
    mt.exe -manifest src\piwiper-gui.manifest -outputresource:piwiper-gui.exe;1
)

if %ERRORLEVEL% neq 0 (
    echo Compilation failed
    pause
    exit /b 1
)

echo Build successful! Executable: piwiper-gui.exe
echo.
echo Run as Administrator for full functionality!
pause

