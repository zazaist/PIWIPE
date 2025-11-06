@echo off
echo Building piwiper GUI with MSVC...

REM Clean old build files to prevent confusion
if exist "piwiper-gui-new.exe" (
    echo Removing old piwiper-gui-new.exe...
    del /F /Q "piwiper-gui-new.exe" >nul 2>&1
)
if exist "piwiper-gui-debug.exe" (
    echo Removing old piwiper-gui-debug.exe...
    del /F /Q "piwiper-gui-debug.exe" >nul 2>&1
)
if exist "main_gui.obj" (
    echo Removing old object files...
    del /F /Q "main_gui.obj" >nul 2>&1
)

REM Load Visual Studio environment
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" (
    echo Loading Visual Studio 2022 Build Tools environment...
    call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" (
    echo Loading Visual Studio 2022 Build Tools environment...
    call "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat"
) else (
    echo Visual Studio Build Tools not found. Please install Visual Studio Build Tools 2022
    pause
    exit /b 1
)

REM Check if Visual Studio is available
where cl >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Visual Studio environment not loaded properly
    pause
    exit /b 1
)

echo Compiling GUI version (without resources for now)...
cl /EHsc /W4 /std:c++17 /I. src\main_gui.cpp /Fe:piwiper-gui-debug.exe user32.lib comctl32.lib gdi32.lib msimg32.lib advapi32.lib shell32.lib /link /SUBSYSTEM:WINDOWS

if %ERRORLEVEL% neq 0 (
    echo Compilation failed
    pause
    exit /b 1
)

REM Embed Administrator manifest
if exist "piwiper-gui-debug.exe" (
    echo Embedding Administrator manifest...
    mt.exe -manifest src\piwiper-gui.manifest -outputresource:piwiper-gui-debug.exe;1
    if %ERRORLEVEL% neq 0 (
        echo Warning: Manifest embedding failed, but executable was created
    )
) else (
    echo ERROR: Executable not found after compilation!
    pause
    exit /b 1
)

REM Verify the executable exists and is recent
if exist "piwiper-gui-debug.exe" (
    echo Build successful! Executable: piwiper-gui-debug.exe
    for %%F in ("piwiper-gui-debug.exe") do echo File size: %%~zF bytes, Modified: %%~tF
) else (
    echo ERROR: Executable was not created!
    pause
    exit /b 1
)
echo.
echo Run as Administrator for full functionality!
pause

