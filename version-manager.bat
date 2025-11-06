@echo off
title PIWIPER Version Manager
color 0B

:menu
cls
echo.
echo ========================================
echo    PIWIPER VERSION MANAGER v1.0
echo ========================================
echo.
echo Current Version Information:
echo ============================
type src\version.h | findstr "#define PIWIPER_VERSION"
echo.
echo Choose action:
echo.
echo [1] Show Current Version
echo [2] Update Version Number
echo [3] Create Git Tag
echo [4] Generate Release Notes
echo [5] Build with Version Info
echo [6] Show Version History
echo [0] Exit
echo.
set /p choice="Enter your choice (0-6): "

if "%choice%"=="1" goto show_version
if "%choice%"=="2" goto update_version
if "%choice%"=="3" goto create_tag
if "%choice%"=="4" goto generate_notes
if "%choice%"=="5" goto build_version
if "%choice%"=="6" goto show_history
if "%choice%"=="0" goto exit
goto menu

:show_version
cls
echo ========================================
echo    CURRENT VERSION INFORMATION
echo ========================================
echo.
echo Reading version.h...
echo.
type src\version.h | findstr "#define PIWIPER_VERSION"
echo.
echo Build Information:
echo - Build Date: %date% %time%
echo - Build Config: Release
echo.
echo Application Information:
type src\version.h | findstr "#define PIWIPER_APP"
echo.
pause
goto menu

:update_version
cls
echo ========================================
echo    UPDATE VERSION NUMBER
echo ========================================
echo.
echo Current version:
type src\version.h | findstr "#define PIWIPER_VERSION_MAJOR"
type src\version.h | findstr "#define PIWIPER_VERSION_MINOR"
type src\version.h | findstr "#define PIWIPER_VERSION_PATCH"
type src\version.h | findstr "#define PIWIPER_VERSION_BUILD"
echo.
set /p major="Enter new MAJOR version (current + 1 for breaking changes): "
set /p minor="Enter new MINOR version (current + 1 for new features): "
set /p patch="Enter new PATCH version (current + 1 for bug fixes): "
set /p build="Enter new BUILD number (current + 1): "

echo.
echo Updating version to %major%.%minor%.%patch%.%build%...
echo.

REM Create backup
copy src\version.h src\version.h.backup

REM Update version numbers (this is a simplified approach)
echo Please manually update src\version.h with the new version numbers.
echo.
echo After updating, run option [5] to build with new version.
echo.
pause
goto menu

:create_tag
cls
echo ========================================
echo    CREATE GIT TAG
echo ========================================
echo.
echo Current version:
type src\version.h | findstr "#define PIWIPER_VERSION_SHORT"
echo.
set /p tag_name="Enter tag name (e.g., v2.1.0): "
set /p tag_message="Enter tag message: "

echo.
echo Creating git tag...
git tag -a %tag_name% -m "%tag_message%"

if %ERRORLEVEL% equ 0 (
    echo Tag created successfully!
    echo.
    echo To push tag to GitHub:
    echo git push origin %tag_name%
) else (
    echo Failed to create tag!
)

echo.
pause
goto menu

:generate_notes
cls
echo ========================================
echo    GENERATE RELEASE NOTES
echo ========================================
echo.
set /p version="Enter version for release notes (e.g., v2.1.0): "
set /p release_date="Enter release date (e.g., 2025-01-17): "

echo Creating release notes...
echo.
echo # PIWIPER %version% Release Notes > RELEASE_NOTES_%version%.md
echo. >> RELEASE_NOTES_%version%.md
echo **Release Date:** %release_date% >> RELEASE_NOTES_%version%.md
echo. >> RELEASE_NOTES_%version%.md
echo ## What's New >> RELEASE_NOTES_%version%.md
echo. >> RELEASE_NOTES_%version%.md
echo - Added comprehensive versioning system >> RELEASE_NOTES_%version%.md
echo - Added About dialog with version information >> RELEASE_NOTES_%version%.md
echo - Enhanced build system with version resources >> RELEASE_NOTES_%version%.md
echo - Improved project structure and documentation >> RELEASE_NOTES_%version%.md
echo. >> RELEASE_NOTES_%version%.md
echo ## Features >> RELEASE_NOTES_%version%.md
echo. >> RELEASE_NOTES_%version%.md
echo - Modern UI with gradient design >> RELEASE_NOTES_%version%.md
echo - Dual progress tracking >> RELEASE_NOTES_%version%.md
echo - Real-time speed display >> RELEASE_NOTES_%version%.md
echo - Verification system >> RELEASE_NOTES_%version%.md
echo - Comprehensive backup system >> RELEASE_NOTES_%version%.md
echo - OS disk protection >> RELEASE_NOTES_%version%.md
echo. >> RELEASE_NOTES_%version%.md
echo ## Technical Details >> RELEASE_NOTES_%version%.md
echo. >> RELEASE_NOTES_%version%.md
echo - Version: %version% >> RELEASE_NOTES_%version%.md
echo - Build Date: %date% %time% >> RELEASE_NOTES_%version%.md
echo - Target Platform: Windows 10/11 >> RELEASE_NOTES_%version%.md
echo - Architecture: x64 >> RELEASE_NOTES_%version%.md
echo. >> RELEASE_NOTES_%version%.md
echo ## Download >> RELEASE_NOTES_%version%.md
echo. >> RELEASE_NOTES_%version%.md
echo - [piwiper-gui.exe](https://github.com/zazaist/PIWIPE/releases/tag/%version%) >> RELEASE_NOTES_%version%.md
echo. >> RELEASE_NOTES_%version%.md
echo ## Installation >> RELEASE_NOTES_%version%.md
echo. >> RELEASE_NOTES_%version%.md
echo 1. Download the executable >> RELEASE_NOTES_%version%.md
echo 2. Run as Administrator >> RELEASE_NOTES_%version%.md
echo 3. Follow the on-screen instructions >> RELEASE_NOTES_%version%.md
echo. >> RELEASE_NOTES_%version%.md
echo **⚠️ WARNING: This tool permanently destroys data!** >> RELEASE_NOTES_%version%.md

echo Release notes created: RELEASE_NOTES_%version%.md
echo.
pause
goto menu

:build_version
cls
echo ========================================
echo    BUILD WITH VERSION INFO
echo ========================================
echo.
echo Building PIWIPER with current version information...
echo.
call build-gui.bat
echo.
echo Build completed with version information embedded.
echo.
pause
goto menu

:show_history
cls
echo ========================================
echo    VERSION HISTORY
echo ========================================
echo.
echo Git tags (releases):
git tag -l --sort=-version:refname
echo.
echo Recent commits:
git log --oneline -10
echo.
pause
goto menu

:exit
echo.
echo Thank you for using PIWIPER Version Manager!
echo.
exit /b 0









