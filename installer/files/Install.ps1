param(
    [Parameter(Mandatory=$true)][string]$GamePath,
    [string]$Destination = (Join-Path $env:LOCALAPPDATA 'RivalsEightLocal\Game'),
    [switch]$ValidateOnly
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
. (Join-Path $PSScriptRoot 'SharedFiles.ps1')
$source = (Resolve-Path -LiteralPath $GamePath).Path.TrimEnd('\')
$target = [IO.Path]::GetFullPath($Destination).TrimEnd('\')
if ($target -eq $source -or $target.StartsWith($source + '\', [StringComparison]::OrdinalIgnoreCase) -or $source.StartsWith($target + '\', [StringComparison]::OrdinalIgnoreCase)) { throw 'The destination must be separate from the original game directory.' }
if (Test-Path -LiteralPath $target) { throw 'Choose a new, nonexistent destination directory. Existing installations are never overwritten.' }
$exe = Join-Path $source 'RivalsofAether.exe'
$originalDll = Join-Path $source 'catch_error_mini.dll'
$expectedExe = '920e149d3bb6acc0c2148e8814af0de2653952e95b4f03bd6b576adfff8d37d8'
$expectedDll = '01c4b8611ceb00d743462543ff341d3b7b5b14abc8c54fc0cbf8c0e01cd90b79'
if ((Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash -ne $expectedExe) { throw 'Unsupported game executable. This preview requires the tested Windows 2.1.9.1 Steam build.' }
if ((Get-FileHash -LiteralPath $originalDll -Algorithm SHA256).Hash -ne $expectedDll) { throw 'The original game DLL differs from the tested version. Use a clean Steam installation.' }
$manifest = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'manifest.json') -Raw | ConvertFrom-Json
foreach ($entry in $manifest.files) {
    $file = Join-Path $PSScriptRoot $entry.path
    if ((Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash -ne $entry.sha256) { throw "Package verification failed: $($entry.path)" }
}
if ($ValidateOnly) { Write-Output 'Source and package verified. No files changed.'; return }
if (Get-Process -Name RivalsofAether -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq $exe }) { throw 'Close the source game before installing.' }
New-Item -ItemType Directory -Path $target | Out-Null
Get-ChildItem -LiteralPath $source -Force | Where-Object { $_.Name -notin @('catch_error_mini.dll','workshop','stages','buddies') } | ForEach-Object { Copy-Item -LiteralPath $_.FullName -Destination $target -Recurse -Force }
if ((Get-FileHash -LiteralPath (Join-Path $target 'RivalsofAether.exe') -Algorithm SHA256).Hash -ne $expectedExe) { throw 'Copied executable verification failed. The incomplete destination has been retained for inspection.' }
Copy-Item -LiteralPath $originalDll -Destination (Join-Path $target 'catch_error_original.dll')
Set-SharedModFile -Source (Join-Path $PSScriptRoot 'mod\catch_error_mini.dll') -Destination (Join-Path $target 'catch_error_mini.dll')
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'Launch.ps1') -Destination $target
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'Launch.cmd') -Destination $target
Set-Content -LiteralPath (Join-Path $target 'hud-layout.txt') -Value 'bottom' -Encoding ASCII
$record = [ordered]@{ version='0.1.3-preview'; gameSha256=$expectedExe; modSha256=(Get-FileHash -LiteralPath (Join-Path $target 'catch_error_mini.dll') -Algorithm SHA256).Hash; source=$source; installed=(Get-Date).ToString('o') }
$record | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $target 'installation.json') -Encoding UTF8
Write-Output "Installed a separate copy at $target. Start it with Launch.cmd. The Steam installation is unchanged."
