param(
    [string]$GamePath,
    [string]$Destination,
    [switch]$UpdateOnly,
    [switch]$ValidateOnly
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$locationFile = Join-Path $env:LOCALAPPDATA 'RivalsEightLocal\installation-path.txt'
if (-not $Destination) {
    $Destination = Join-Path $env:LOCALAPPDATA 'RivalsEightLocal\Game'
    if (Test-Path -LiteralPath $locationFile) {
        $saved = (Get-Content -LiteralPath $locationFile -Raw).Trim()
        if ($saved -and (Test-Path -LiteralPath (Join-Path $saved 'installation.json'))) { $Destination = $saved }
    }
}
if ($UpdateOnly -and -not (Test-Path -LiteralPath (Join-Path $Destination 'installation.json'))) {
    $Destination = (Read-Host 'Paste the folder containing your installed mod, then press Enter').Trim().Trim('"')
    if (-not $Destination -or -not (Test-Path -LiteralPath (Join-Path $Destination 'installation.json'))) {
        throw 'No installed mod found in that folder. For a first installation, double-click Setup.cmd.'
    }
}
if (Test-Path -LiteralPath (Join-Path $Destination 'installation.json')) {
    Write-Output 'Updating your existing mod installation...'
    & (Join-Path $PSScriptRoot 'Update.ps1') -Destination $Destination -ValidateOnly:$ValidateOnly
} else {
    if (-not $GamePath) {
        $steamRoots = @()
        foreach ($key in @('HKCU:\Software\Valve\Steam', 'HKLM:\SOFTWARE\WOW6432Node\Valve\Steam')) {
            $info = Get-ItemProperty -LiteralPath $key -ErrorAction SilentlyContinue
            if ($info) {
                foreach ($field in @('SteamPath','InstallPath')) {
                    if ($info.PSObject.Properties[$field]) { $steamRoots += [string]$info.$field }
                }
            }
        }
        if (${env:ProgramFiles(x86)}) { $steamRoots += (Join-Path ${env:ProgramFiles(x86)} 'Steam') }
        $libraries = @($steamRoots)
        foreach ($root in @($steamRoots | Select-Object -Unique)) {
            $vdf = Join-Path $root 'steamapps\libraryfolders.vdf'
            if (Test-Path -LiteralPath $vdf) {
                foreach ($match in [regex]::Matches((Get-Content -LiteralPath $vdf -Raw), '"path"\s+"([^"]+)"')) {
                    $libraries += $match.Groups[1].Value.Replace('\\','\')
                }
            }
        }
        foreach ($library in @($libraries | Select-Object -Unique)) {
            $candidate = Join-Path $library 'steamapps\common\Rivals of Aether'
            if ((Test-Path -LiteralPath (Join-Path $candidate 'RivalsofAether.exe')) -and (Test-Path -LiteralPath (Join-Path $candidate 'catch_error_mini.dll'))) {
                $GamePath = $candidate
                break
            }
        }
        if (-not $GamePath) {
            Write-Output 'Open Steam > Library > right-click Rivals of Aether > Manage > Browse local files.'
            $GamePath = (Read-Host 'Copy that folder address here, then press Enter').Trim().Trim('"')
            if (-not $GamePath) { throw 'No game folder selected. Setup cancelled.' }
        }
    }
    Write-Output "Installing from $GamePath"
    & (Join-Path $PSScriptRoot 'Install.ps1') -GamePath $GamePath -Destination $Destination -ValidateOnly:$ValidateOnly
}
if ($ValidateOnly) { return }
New-Item -ItemType Directory -Path (Split-Path $locationFile -Parent) -Force | Out-Null
Set-Content -LiteralPath $locationFile -Value ([IO.Path]::GetFullPath($Destination)) -Encoding UTF8
Write-Output ''
Write-Output 'Setup complete. Double-click Launch.cmd in this extracted folder to play.'
Write-Output 'Keep Steam running. In the game, choose Local > Versus.'
