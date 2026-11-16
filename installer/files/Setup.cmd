@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Setup.ps1"
set "setupResult=%errorlevel%"
echo.
pause
exit /b %setupResult%
