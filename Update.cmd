@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Setup.ps1" -UpdateOnly
set "setupResult=%errorlevel%"
echo.
pause
exit /b %setupResult%
