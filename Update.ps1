param(
    [string]$Destination = (Join-Path $env:LOCALAPPDATA 'RivalsEightLocal\Game'),
    [switch]$ValidateOnly
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$target = (Resolve-Path -LiteralPath $Destination).Path
$recordPath = Join-Path $target 'installation.json'
if (-not (Test-Path -LiteralPath $recordPath)) { throw 'This directory is not an installed Rivals Eight Local copy. Use Install.ps1 for a new installation.' }
$record = Get-Content -LiteralPath $recordPath -Raw | ConvertFrom-Json
$exe = Join-Path $target 'RivalsofAether.exe'
$mod = Join-Path $target 'catch_error_mini.dll'
if ((Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash -ne $record.gameSha256) { throw 'Installed game verification failed.' }
if ($record.gameSha256 -ne '920e149d3bb6acc0c2148e8814af0de2653952e95b4f03bd6b576adfff8d37d8') { throw 'Unsupported installed game version.' }
if ((Get-FileHash -LiteralPath $mod -Algorithm SHA256).Hash -ne $record.modSha256) { throw 'Installed mod verification failed.' }
$manifest = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'manifest.json') -Raw | ConvertFrom-Json
foreach ($entry in $manifest.files) {
    if ((Get-FileHash -LiteralPath (Join-Path $PSScriptRoot $entry.path) -Algorithm SHA256).Hash -ne $entry.sha256) { throw "Package verification failed: $($entry.path)" }
}
if ($ValidateOnly) { Write-Output 'Installed copy and update package verified. No files changed.'; return }
if (Get-Process -Name RivalsofAether -ErrorAction SilentlyContinue) { throw 'Close Rivals of Aether before updating.' }
$backup = Join-Path $target ('ModBackups\' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
New-Item -ItemType Directory -Path $backup | Out-Null
foreach ($name in @('catch_error_mini.dll','Launch.ps1','Launch.cmd','installation.json')) {
    Copy-Item -LiteralPath (Join-Path $target $name) -Destination (Join-Path $backup $name)
}
try {
    Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'mod\catch_error_mini.dll') -Destination $mod -Force
    foreach ($name in @('Launch.ps1','Launch.cmd')) { Copy-Item -LiteralPath (Join-Path $PSScriptRoot $name) -Destination (Join-Path $target $name) -Force }
    $record.version = $manifest.version
    $record.modSha256 = (Get-FileHash -LiteralPath $mod -Algorithm SHA256).Hash
    $record | ConvertTo-Json | Set-Content -LiteralPath $recordPath -Encoding UTF8
} catch {
    foreach ($name in @('catch_error_mini.dll','Launch.ps1','Launch.cmd','installation.json')) { Copy-Item -LiteralPath (Join-Path $backup $name) -Destination (Join-Path $target $name) -Force }
    throw
}
Write-Output "Updated to $($manifest.version). Previous mod files: $backup. Start the copied game with Launch.cmd."
