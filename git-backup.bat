@echo off
echo ========================================
echo    PIWIPER GIT BACKUP SCRIPT
echo ========================================
echo.

REM Check if git is available
where git >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: Git not found! Please install Git first.
    pause
    exit /b 1
)

echo [1/4] Checking git status...
git status --porcelain
if %ERRORLEVEL% neq 0 (
    echo ERROR: Not in a git repository!
    pause
    exit /b 1
)

echo.
echo [2/4] Adding all changes...
git add .

echo [3/4] Committing changes...
set /p COMMIT_MSG="Enter commit message (or press Enter for auto): "
if "%COMMIT_MSG%"=="" (
    set COMMIT_MSG=Auto backup - %date% %time%
)
git commit -m "%COMMIT_MSG%"

echo.
echo [4/4] Pushing to GitHub...
git push origin main

if %ERRORLEVEL% equ 0 (
    echo.
    echo ========================================
    echo    GIT BACKUP COMPLETED SUCCESSFULLY!
    echo ========================================
    echo.
    echo Repository: https://github.com/zazaist/PIWIPE
    echo Last commit: 
    git log -1 --oneline
) else (
    echo.
    echo ========================================
    echo    GIT BACKUP FAILED!
    echo ========================================
    echo Please check your internet connection and GitHub credentials.
)

echo.
pause






