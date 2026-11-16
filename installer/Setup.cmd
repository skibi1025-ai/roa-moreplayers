@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0files\Setup.ps1"
set "setupResult=%errorlevel%"
echo.
pause
exit /b %setupResult%
