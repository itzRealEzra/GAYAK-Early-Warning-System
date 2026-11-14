@echo off
setlocal
cd /d "%~dp0"

if not exist "requirements.txt" (
    echo ERROR: requirements.txt was not found.
    echo Run this installer from the extracted GAYAK project folder.
    goto :failed
)

echo ========================================
echo GAYAK - Local Dashboard Installer
echo ========================================
echo.

set "GAYAK_PYTHON_CMD="
where py >nul 2>&1
if not errorlevel 1 (
    py -3 --version >nul 2>&1
    if not errorlevel 1 set "GAYAK_PYTHON_CMD=py -3"
)

if not defined GAYAK_PYTHON_CMD (
    where python >nul 2>&1
    if not errorlevel 1 (
        python --version >nul 2>&1
        if not errorlevel 1 set "GAYAK_PYTHON_CMD=python"
    )
)

if not defined GAYAK_PYTHON_CMD (
    echo ERROR: Python 3 was not found.
    echo Install Python for Windows from https://www.python.org/downloads/windows/
    echo Then run this installer again.
    goto :failed
)

echo [INFO] Creating or checking the project virtual environment...
if not exist ".venv\Scripts\python.exe" (
    %GAYAK_PYTHON_CMD% -m venv .venv
    if errorlevel 1 (
        echo ERROR: Could not create .venv.
        echo Reinstall Python with the venv component enabled, then retry.
        goto :failed
    )
)

echo [INFO] Downloading and installing Flask and PySerial...
".venv\Scripts\python.exe" -m pip install -r requirements.txt
if errorlevel 1 (
    echo ERROR: Dependency installation failed.
    echo Check your internet connection and try again.
    goto :failed
)

echo.
echo [OK] GAYAK dashboard dependencies are installed in .venv.
echo Run start_gayak.bat to launch the dashboard.
if /I not "%~1"=="/S" pause
exit /b 0

:failed
echo.
if /I not "%~1"=="/S" pause
exit /b 1
