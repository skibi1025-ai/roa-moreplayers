$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$record = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'installation.json') -Raw | ConvertFrom-Json
$exe = Join-Path $PSScriptRoot 'RivalsofAether.exe'
if ((Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash -ne $record.gameSha256) { throw 'Game version mismatch. Reinstall into a new directory using the supported game version.' }
if ((Get-FileHash -LiteralPath (Join-Path $PSScriptRoot 'catch_error_mini.dll') -Algorithm SHA256).Hash -ne $record.modSha256) { throw 'Mod DLL verification failed. Re-extract the package and reinstall.' }
if (Get-Process -Name RivalsofAether -ErrorAction SilentlyContinue) { throw 'Close other Rivals of Aether windows before launching the mod.' }
$savePath = Join-Path $env:LOCALAPPDATA 'RivalsofAether'
$backupRoot = Join-Path $env:LOCALAPPDATA 'RivalsEightLocal\SaveBackups'
$backup = Join-Path $backupRoot (Get-Date -Format 'yyyyMMdd-HHmmss-fff')
if (Test-Path -LiteralPath $savePath) {
    New-Item -ItemType Directory -Path $backup -Force | Out-Null
    Get-ChildItem -LiteralPath $savePath -Force | Where-Object { $_.Name -notin @('workshop','stages','buddies') } | ForEach-Object { Copy-Item -LiteralPath $_.FullName -Destination $backup -Recurse -Force }
    Write-Output "Shared save files backed up to $backup"
}
Write-Output 'Experimental local-only build. Use Local > Versus. Steam Workshop subscription alone does not install this mod.'
Start-Process -FilePath $exe -WorkingDirectory $PSScriptRoot -WindowStyle Hidden
