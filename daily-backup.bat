@echo off
REM ========================================
REM    PIWIPER DAILY AUTO-BACKUP SCRIPT
REM ========================================
REM This script can be scheduled with Windows Task Scheduler
REM for automatic daily backups

setlocal enabledelayedexpansion

REM Set paths
set PROJECT_DIR=%~dp0
set BACKUP_ROOT=%PROJECT_DIR%backups\daily
set LOG_FILE=%PROJECT_DIR%backup.log

REM Create backup directory
if not exist "%BACKUP_ROOT%" mkdir "%BACKUP_ROOT%"

REM Get current date
for /f "tokens=2 delims==" %%a in ('wmic OS Get localdatetime /value') do set "dt=%%a"
set "YY=%dt:~2,2%" & set "YYYY=%dt:~0,4%" & set "MM=%dt:~4,2%" & set "DD=%dt:~6,2%"
set "HH=%dt:~8,2%" & set "Min=%dt:~10,2%" & set "Sec=%dt:~12,2%"
set "TIMESTAMP=%YYYY%%MM%%DD%_%HH%%Min%"

echo [%date% %time%] Starting daily backup... >> "%LOG_FILE%"

REM Create today's backup folder
set BACKUP_FOLDER=%BACKUP_ROOT%\PIWIPER_Daily_%TIMESTAMP%
mkdir "%BACKUP_FOLDER%"

REM Backup source files
echo [%date% %time%] Backing up source files... >> "%LOG_FILE%"
xcopy "src" "%BACKUP_FOLDER%\src\" /E /I /Y /Q
xcopy "*.bat" "%BACKUP_FOLDER%\" /Y /Q
xcopy "*.txt" "%BACKUP_FOLDER%\" /Y /Q
xcopy "*.md" "%BACKUP_FOLDER%\" /Y /Q
xcopy "*.ico" "%BACKUP_FOLDER%\" /Y /Q
xcopy "*.png" "%BACKUP_FOLDER%\" /Y /Q
xcopy "CMakeLists.txt" "%BACKUP_FOLDER%\" /Y /Q
xcopy ".gitignore" "%BACKUP_FOLDER%\" /Y /Q

REM Backup executables if they exist
if exist "*.exe" (
    echo [%date% %time%] Backing up executables... >> "%LOG_FILE%"
    if not exist "%BACKUP_FOLDER%\bin" mkdir "%BACKUP_FOLDER%\bin"
    xcopy "*.exe" "%BACKUP_FOLDER%\bin\" /Y /Q
)

REM Git status backup
echo [%date% %time%] Recording git status... >> "%LOG_FILE%"
echo Daily Backup - %date% %time% > "%BACKUP_FOLDER%\BACKUP_INFO.txt"
echo ================================ >> "%BACKUP_FOLDER%\BACKUP_INFO.txt"
echo. >> "%BACKUP_FOLDER%\BACKUP_INFO.txt"
git status --porcelain >> "%BACKUP_FOLDER%\BACKUP_INFO.txt" 2>nul
echo. >> "%BACKUP_FOLDER%\BACKUP_INFO.txt"
echo Last 5 commits: >> "%BACKUP_FOLDER%\BACKUP_INFO.txt"
git log -5 --oneline >> "%BACKUP_FOLDER%\BACKUP_INFO.txt" 2>nul

REM Clean old backups (keep last 7 days)
echo [%date% %time%] Cleaning old backups... >> "%LOG_FILE%"
forfiles /p "%BACKUP_ROOT%" /m "PIWIPER_Daily_*" /d -7 /c "cmd /c if @isdir==TRUE rmdir /s /q @path" 2>nul

REM Create summary
echo [%date% %time%] Creating backup summary... >> "%LOG_FILE%"
echo Daily Backup Summary > "%BACKUP_FOLDER%\SUMMARY.txt"
echo ==================== >> "%BACKUP_FOLDER%\SUMMARY.txt"
echo Date: %date% %time% >> "%BACKUP_FOLDER%\SUMMARY.txt"
echo Source: %PROJECT_DIR% >> "%BACKUP_FOLDER%\SUMMARY.txt"
echo Backup: %BACKUP_FOLDER% >> "%BACKUP_FOLDER%\SUMMARY.txt"
echo. >> "%BACKUP_FOLDER%\SUMMARY.txt"
echo Files backed up: >> "%BACKUP_FOLDER%\SUMMARY.txt"
dir "%BACKUP_FOLDER%" /B >> "%BACKUP_FOLDER%\SUMMARY.txt"

REM Log completion
echo [%date% %time%] Daily backup completed successfully! >> "%LOG_FILE%"
echo [%date% %time%] Backup location: %BACKUP_FOLDER% >> "%LOG_FILE%"

REM Optional: Auto-commit to git if there are changes
git status --porcelain | findstr /R "." >nul
if %ERRORLEVEL% equ 0 (
    echo [%date% %time%] Auto-committing changes to git... >> "%LOG_FILE%"
    git add . >> "%LOG_FILE%" 2>&1
    git commit -m "Daily auto-backup - %date% %time%" >> "%LOG_FILE%" 2>&1
    git push origin main >> "%LOG_FILE%" 2>&1
    echo [%date% %time%] Git auto-commit completed. >> "%LOG_FILE%"
)

echo [%date% %time%] Daily backup process finished. >> "%LOG_FILE%"
echo. >> "%LOG_FILE%"

endlocal
