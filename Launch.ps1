param(
    [string]$Destination,
    [switch]$ValidateOnly
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$installRoot = $PSScriptRoot
if (-not (Test-Path -LiteralPath (Join-Path $installRoot 'installation.json'))) {
    if (-not $Destination) {
        $Destination = Join-Path $env:LOCALAPPDATA 'RivalsEightLocal\Game'
        $locationFile = Join-Path $env:LOCALAPPDATA 'RivalsEightLocal\installation-path.txt'
        if (Test-Path -LiteralPath $locationFile) {
            $saved = (Get-Content -LiteralPath $locationFile -Raw).Trim()
            if ($saved -and (Test-Path -LiteralPath (Join-Path $saved 'installation.json'))) { $Destination = $saved }
        }
    }
    if (-not (Test-Path -LiteralPath (Join-Path $Destination 'installation.json'))) {
        throw 'The mod is not installed yet. Extract the whole ZIP, then double-click Setup.cmd. When setup finishes, double-click Launch.cmd.'
    }
    $installRoot = $Destination
}
$record = Get-Content -LiteralPath (Join-Path $installRoot 'installation.json') -Raw | ConvertFrom-Json
$exe = Join-Path $installRoot 'RivalsofAether.exe'
if ((Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash -ne $record.gameSha256) { throw 'Game version mismatch. Reinstall into a new directory using the supported game version.' }
if ((Get-FileHash -LiteralPath (Join-Path $installRoot 'catch_error_mini.dll') -Algorithm SHA256).Hash -ne $record.modSha256) { throw 'Mod DLL verification failed. Re-extract the package and reinstall.' }
if (Get-Process -Name RivalsofAether -ErrorAction SilentlyContinue) { throw 'Close other Rivals of Aether windows before launching the mod.' }
if ($ValidateOnly) { Write-Output 'Launcher and installed files verified. No game opened.'; return }
$savePath = Join-Path $env:LOCALAPPDATA 'RivalsofAether'
$backupRoot = Join-Path $env:LOCALAPPDATA 'RivalsEightLocal\SaveBackups'
$backup = Join-Path $backupRoot (Get-Date -Format 'yyyyMMdd-HHmmss-fff')
if (Test-Path -LiteralPath $savePath) {
    New-Item -ItemType Directory -Path $backup -Force | Out-Null
    Get-ChildItem -LiteralPath $savePath -Force | Where-Object { $_.Name -notin @('workshop','stages','buddies') } | ForEach-Object { Copy-Item -LiteralPath $_.FullName -Destination $backup -Recurse -Force }
    Write-Output "Shared save files backed up to $backup"
}
Write-Output 'Experimental local-only build. Use Local > Versus. Steam Workshop subscription alone does not install this mod.'
Start-Process -FilePath $exe -WorkingDirectory $installRoot -WindowStyle Hidden
