@echo off
echo ========================================
echo    PIWIPER PROJECT BACKUP SCRIPT
echo ========================================
echo.

REM Set backup directory
set BACKUP_DIR=%~dp0backups
set TIMESTAMP=%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%
set TIMESTAMP=%TIMESTAMP: =0%

echo Creating backup directory...
if not exist "%BACKUP_DIR%" mkdir "%BACKUP_DIR%"

echo.
echo [1/5] Creating timestamped backup folder...
set BACKUP_FOLDER=%BACKUP_DIR%\PIWIPER_Backup_%TIMESTAMP%
mkdir "%BACKUP_FOLDER%"

echo [2/5] Copying source files...
xcopy "src" "%BACKUP_FOLDER%\src\" /E /I /Y
xcopy "*.bat" "%BACKUP_FOLDER%\" /Y
xcopy "*.txt" "%BACKUP_FOLDER%\" /Y
xcopy "*.md" "%BACKUP_FOLDER%\" /Y
xcopy "*.ico" "%BACKUP_FOLDER%\" /Y
xcopy "*.png" "%BACKUP_FOLDER%\" /Y
xcopy "CMakeLists.txt" "%BACKUP_FOLDER%\" /Y
xcopy ".gitignore" "%BACKUP_FOLDER%\" /Y

echo [3/5] Copying compiled executables (if exist)...
if not exist "%BACKUP_FOLDER%\bin" mkdir "%BACKUP_FOLDER%\bin"
if exist "*.exe" (
    for %%f in (*.exe) do (
        copy "%%f" "%BACKUP_FOLDER%\bin\" /Y >nul 2>&1
    )
)

echo [4/5] Creating backup info file...
echo PIWIPER Project Backup > "%BACKUP_FOLDER%\BACKUP_INFO.txt"
echo Created: %date% %time% >> "%BACKUP_FOLDER%\BACKUP_INFO.txt"
echo Source: %~dp0 >> "%BACKUP_FOLDER%\BACKUP_INFO.txt"
echo Git Status: >> "%BACKUP_FOLDER%\BACKUP_INFO.txt"
git status --porcelain >> "%BACKUP_FOLDER%\BACKUP_INFO.txt" 2>nul || echo Not a git repository >> "%BACKUP_FOLDER%\BACKUP_INFO.txt"
echo. >> "%BACKUP_FOLDER%\BACKUP_INFO.txt"
echo Git Last Commit: >> "%BACKUP_FOLDER%\BACKUP_INFO.txt"
git log -1 --oneline >> "%BACKUP_FOLDER%\BACKUP_INFO.txt" 2>nul || echo No commits found >> "%BACKUP_FOLDER%\BACKUP_INFO.txt"

echo [5/5] Creating ZIP archive...
cd "%BACKUP_DIR%"
powershell -NoProfile -Command "Compress-Archive -Path 'PIWIPER_Backup_%TIMESTAMP%' -DestinationPath 'PIWIPER_Backup_%TIMESTAMP%.zip' -Force" 2>nul
cd "%~dp0"

echo.
echo ========================================
echo    BACKUP COMPLETED SUCCESSFULLY!
echo ========================================
echo.
echo Backup Location: %BACKUP_FOLDER%
echo ZIP Archive: %BACKUP_DIR%\PIWIPER_Backup_%TIMESTAMP%.zip
echo.
echo Files backed up:
dir "%BACKUP_FOLDER%" /B 2>nul
echo.
echo Total backup size:
powershell -NoProfile -Command "$size = (Get-ChildItem '%BACKUP_FOLDER%' -Recurse -ErrorAction SilentlyContinue | Measure-Object -Property Length -Sum).Sum; Write-Host ([math]::Round($size/1MB,2)) 'MB'" 2>nul
echo.
echo Backup completed! Press any key to continue...
timeout /t 2 >nul






