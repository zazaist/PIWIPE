@echo off
title PIWIPER Backup Manager
color 0A

:menu
cls
echo.
echo ========================================
echo    PIWIPER BACKUP MANAGER v1.0
echo ========================================
echo.
echo Choose backup option:
echo.
echo [1] Full Project Backup (Manual)
echo [2] Git Backup (Push to GitHub)
echo [3] WipeLogs Backup
echo [4] Daily Auto-Backup
echo [5] View Backup History
echo [6] Restore from Backup
echo [7] Clean Old Backups
echo [8] Backup Settings
echo [0] Exit
echo.
set /p choice="Enter your choice (0-8): "

if "%choice%"=="1" goto full_backup
if "%choice%"=="2" goto git_backup
if "%choice%"=="3" goto wipelogs_backup
if "%choice%"=="4" goto daily_backup
if "%choice%"=="5" goto view_history
if "%choice%"=="6" goto restore_backup
if "%choice%"=="7" goto clean_backups
if "%choice%"=="8" goto backup_settings
if "%choice%"=="0" goto exit
goto menu

:full_backup
cls
echo Running Full Project Backup...
call backup-project.bat
pause
goto menu

:git_backup
cls
echo Running Git Backup...
call git-backup.bat
pause
goto menu

:wipelogs_backup
cls
echo Running WipeLogs Backup...
call backup-wipelogs.bat
pause
goto menu

:daily_backup
cls
echo Running Daily Auto-Backup...
call daily-backup.bat
echo Daily backup completed!
pause
goto menu

:view_history
cls
echo ========================================
echo    BACKUP HISTORY
echo ========================================
echo.
if exist "backups" (
    echo Recent backups:
    echo.
    dir "backups" /B /O:D
    echo.
    echo Backup details:
    for /f %%i in ('dir "backups" /B /O:D') do (
        echo.
        echo === %%i ===
        if exist "backups\%%i\BACKUP_INFO.txt" (
            type "backups\%%i\BACKUP_INFO.txt"
        )
    )
) else (
    echo No backups found.
)
echo.
pause
goto menu

:restore_backup
cls
echo ========================================
echo    RESTORE FROM BACKUP
echo ========================================
echo.
if not exist "backups" (
    echo No backups found.
    pause
    goto menu
)

echo Available backups:
dir "backups" /B /O:D
echo.
set /p restore_choice="Enter backup folder name to restore: "

if not exist "backups\%restore_choice%" (
    echo Backup not found!
    pause
    goto menu
)

echo.
echo WARNING: This will overwrite current files!
set /p confirm="Are you sure? (y/N): "
if /i not "%confirm%"=="y" goto menu

echo Restoring from backup...
xcopy "backups\%restore_choice%\*" "." /E /I /Y
echo Restore completed!
pause
goto menu

:clean_backups
cls
echo ========================================
echo    CLEAN OLD BACKUPS
echo ========================================
echo.
set /p days="Enter number of days to keep (default 30): "
if "%days%"=="" set days=30

echo Cleaning backups older than %days% days...
forfiles /p "backups" /m "*" /d -%days% /c "cmd /c if @isdir==TRUE rmdir /s /q @path" 2>nul
echo Cleanup completed!
pause
goto menu

:backup_settings
cls
echo ========================================
echo    BACKUP SETTINGS
echo ========================================
echo.
echo Current settings:
echo - Backup directory: %~dp0backups
echo - Git repository: https://github.com/zazaist/PIWIPE
echo - Daily backup retention: 7 days
echo - Full backup retention: 30 days
echo.
echo Settings cannot be modified from this interface.
echo Edit the backup scripts directly to change settings.
echo.
pause
goto menu

:exit
echo.
echo Thank you for using PIWIPER Backup Manager!
echo.
exit /b 0
