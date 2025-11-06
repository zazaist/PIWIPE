@echo off
echo ========================================
echo    WIPELOGS BACKUP SCRIPT
echo ========================================
echo.

REM Set backup directory
set BACKUP_DIR=%~dp0backups\WipeLogs
set TIMESTAMP=%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%
set TIMESTAMP=%TIMESTAMP: =0%

echo Creating WipeLogs backup directory...
if not exist "%BACKUP_DIR%" mkdir "%BACKUP_DIR%"

echo.
echo [1/3] Checking for WipeLogs directories...

REM Check common WipeLogs locations
set FOUND_LOGS=0

if exist "C:\WipeLogs" (
    echo Found: C:\WipeLogs
    set FOUND_LOGS=1
    set SOURCE_LOG=C:\WipeLogs
)

if exist "D:\WipeLogs" (
    echo Found: D:\WipeLogs
    set FOUND_LOGS=1
    set SOURCE_LOG=D:\WipeLogs
)

if exist "E:\WipeLogs" (
    echo Found: E:\WipeLogs
    set FOUND_LOGS=1
    set SOURCE_LOG=E:\WipeLogs
)

if exist ".\WipeLogs" (
    echo Found: .\WipeLogs
    set FOUND_LOGS=1
    set SOURCE_LOG=.\WipeLogs
)

if %FOUND_LOGS%==0 (
    echo No WipeLogs directories found.
    echo Creating sample structure...
    mkdir "%BACKUP_DIR%\Sample_WipeLogs"
    echo Sample WipeLogs backup created.
    goto :end
)

echo.
echo [2/3] Copying WipeLogs...
set BACKUP_FOLDER=%BACKUP_DIR%\WipeLogs_Backup_%TIMESTAMP%
mkdir "%BACKUP_FOLDER%"

xcopy "%SOURCE_LOG%\*" "%BACKUP_FOLDER%\" /E /I /Y

echo.
echo [3/3] Creating summary report...
echo WipeLogs Backup Report > "%BACKUP_FOLDER%\BACKUP_REPORT.txt"
echo ======================== >> "%BACKUP_FOLDER%\BACKUP_REPORT.txt"
echo. >> "%BACKUP_FOLDER%\BACKUP_REPORT.txt"
echo Backup Date: %date% %time% >> "%BACKUP_FOLDER%\BACKUP_REPORT.txt"
echo Source: %SOURCE_LOG% >> "%BACKUP_FOLDER%\BACKUP_REPORT.txt"
echo Destination: %BACKUP_FOLDER% >> "%BACKUP_FOLDER%\BACKUP_REPORT.txt"
echo. >> "%BACKUP_FOLDER%\BACKUP_REPORT.txt"
echo Files backed up: >> "%BACKUP_FOLDER%\BACKUP_REPORT.txt"
dir "%BACKUP_FOLDER%\*.txt" /B >> "%BACKUP_FOLDER%\BACKUP_REPORT.txt" 2>nul

echo.
echo ========================================
echo    WIPELOGS BACKUP COMPLETED!
echo ========================================
echo.
echo Source: %SOURCE_LOG%
echo Backup: %BACKUP_FOLDER%
echo.
echo Files backed up:
dir "%BACKUP_FOLDER%\*.txt" /B 2>nul
echo.

:end
echo Creating ZIP archive...
cd "%BACKUP_DIR%"
if exist "WipeLogs_Backup_%TIMESTAMP%" (
    powershell -Command "Compress-Archive -Path 'WipeLogs_Backup_%TIMESTAMP%' -DestinationPath 'WipeLogs_Backup_%TIMESTAMP%.zip' -Force"
    echo ZIP Archive: %BACKUP_DIR%\WipeLogs_Backup_%TIMESTAMP%.zip
)
cd "%~dp0"

echo.
pause






