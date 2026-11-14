@echo off
setlocal
cd /d "%~dp0"
title GAYAK - Local Monitoring Dashboard
color 0B
cls

:: Dashboard port (override with: set GAYAK_PORT=5001)
if not defined GAYAK_PORT set "GAYAK_PORT=5000"

echo.
echo  ============================================================
echo                         G A Y A K
echo                 EARLY WARNING SYSTEM
echo  ============================================================
echo   Local Sensor Dashboard
echo   Developer: Ezra
echo   Address:   http://127.0.0.1:%GAYAK_PORT%
echo  ------------------------------------------------------------
echo   Initializing dashboard services...
echo.

:: ------------------------------------------------------------------
:: 1. Check / install the project virtual environment
:: ------------------------------------------------------------------
if not exist ".venv\Scripts\python.exe" (
    echo [INFO] Project environment is missing. Running the installer...
    call install_gayak.bat /S
    if errorlevel 1 (
        echo ERROR: Installation did not complete. Review the message above.
        pause
        exit /b 1
    )
)

".venv\Scripts\python.exe" -c "import flask, serial" >nul 2>&1
if errorlevel 1 (
    echo [INFO] Required packages are missing. Updating the project environment...
    call install_gayak.bat /S
    if errorlevel 1 (
        echo ERROR: Installation did not complete. Review the message above.
        pause
        exit /b 1
    )
)
echo [READY] Python environment is ready.

:: ------------------------------------------------------------------
:: 2. Check the dashboard port is free.
::    Only a LISTENING socket on exactly this port counts. The old check
::    matched any line containing ":5000", which also hit ":50001",
::    ":55000" and leftover TIME_WAIT / foreign-address entries.
:: ------------------------------------------------------------------
set "PORT_PID="
for /f "tokens=5" %%P in ('netstat -ano ^| findstr /R /C:":%GAYAK_PORT% .*LISTENING"') do set "PORT_PID=%%P"
if defined PORT_PID (
    echo WARNING: Port %GAYAK_PORT% is already in use by PID %PORT_PID%:
    tasklist /FI "PID eq %PORT_PID%"
    echo.
    echo Close that program ^(taskkill /F /PID %PORT_PID%^), or use another port:
    echo     set GAYAK_PORT=5001
    echo     start_gayak.bat
    echo.
    pause
    exit /b 1
)
echo [READY] Dashboard port %GAYAK_PORT% is available.

:: ------------------------------------------------------------------
:: 3. Open the dashboard full screen (Edge or Chrome app window), falling
::    back to the default browser. Runs in the background after a short delay.
:: ------------------------------------------------------------------
set "GAYAK_URL=http://127.0.0.1:%GAYAK_PORT%"
echo [INFO] Opening the dashboard in a full-screen app window shortly.
echo        Press F11 to leave full screen.
start "" powershell -NoProfile -WindowStyle Hidden -Command "Start-Sleep -Seconds 4; $u=$env:GAYAK_URL; $e=@($env:ProgramFiles+'\Microsoft\Edge\Application\msedge.exe', ${env:ProgramFiles(x86)}+'\Microsoft\Edge\Application\msedge.exe', $env:ProgramFiles+'\Google\Chrome\Application\chrome.exe') | Where-Object { Test-Path $_ } | Select-Object -First 1; if ($e) { Start-Process $e -ArgumentList ('--app='+$u),'--start-fullscreen' } else { Start-Process $u }"

:: ------------------------------------------------------------------
:: 4. Start Flask application (foreground so console stays available)
:: ------------------------------------------------------------------
echo.
echo  ------------------------------------------------------------
echo   GAYAK is starting at http://127.0.0.1:%GAYAK_PORT%
echo   Press Ctrl+C to stop the dashboard server.
echo   Keep this console open while the dashboard is running.
echo  ------------------------------------------------------------
echo.
".venv\Scripts\python.exe" app.py

endlocal
