@echo off
setlocal

:: Dashboard port (override with: set GAYAK_PORT=5001)
if not defined GAYAK_PORT set "GAYAK_PORT=5000"

echo ========================================
echo GAYAK - Hazard Monitoring System
echo Local dashboard: http://127.0.0.1:%GAYAK_PORT%
echo ========================================
echo.

:: ------------------------------------------------------------------
:: 1. Check Python availability
:: ------------------------------------------------------------------
where python >nul
if errorlevel 1 (
    echo ERROR: Python is not installed or not found on your PATH.
    echo Please install Python 3 and add it to PATH during installation.
    echo Get Python at https://www.python.org then run this script again.
    echo.
    pause
    exit /b 1
)
echo [OK] Python found.

:: ------------------------------------------------------------------
:: 2. Install / verify Python dependencies
:: ------------------------------------------------------------------
echo.
echo Installing dependencies from requirements.txt (offline if already cached)...
pip install -r requirements.txt >nul
if errorlevel 1 (
    echo ERROR: Failed to install Python dependencies.
    echo Make sure you have an internet connection once to download Flask and
    echo PySerial, or install them manually: pip install flask pyserial
    echo.
    pause
    exit /b 1
)
echo [OK] Dependencies ready.

:: ------------------------------------------------------------------
:: 3. Check the dashboard port is free.
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
echo [OK] Port %GAYAK_PORT% is free.

:: ------------------------------------------------------------------
:: 4. Open the dashboard full screen (Edge or Chrome app window), falling
::    back to the default browser. Runs in the background after a short delay.
:: ------------------------------------------------------------------
set "GAYAK_URL=http://127.0.0.1:%GAYAK_PORT%"
echo [INFO] Dashboard opens full screen in a few seconds (press F11 to leave full screen).
start "" powershell -NoProfile -WindowStyle Hidden -Command "Start-Sleep -Seconds 4; $u=$env:GAYAK_URL; $e=@($env:ProgramFiles+'\Microsoft\Edge\Application\msedge.exe', ${env:ProgramFiles(x86)}+'\Microsoft\Edge\Application\msedge.exe', $env:ProgramFiles+'\Google\Chrome\Application\chrome.exe') | Where-Object { Test-Path $_ } | Select-Object -First 1; if ($e) { Start-Process $e -ArgumentList ('--app='+$u),'--start-fullscreen' } else { Start-Process $u }"

:: ------------------------------------------------------------------
:: 5. Start Flask application (foreground so console stays available)
:: ------------------------------------------------------------------
echo.
echo Starting GAYAK dashboard server...
echo Press Ctrl+C to stop. Keep this console open for troubleshooting.
echo.
set "FLASK_ENV=development"
python app.py

endlocal
