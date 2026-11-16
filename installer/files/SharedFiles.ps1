function Set-SharedModFile {
    param([string]$Source, [string]$Destination)
    $sourceFile = (Resolve-Path -LiteralPath $Source).Path
    $staged = Join-Path (Split-Path $Destination -Parent) ('.mod-' + [guid]::NewGuid().ToString('N') + '.tmp')
    try {
        try {
            New-Item -ItemType HardLink -Path $staged -Value $sourceFile -ErrorAction Stop | Out-Null
            Write-Output 'Mod file shares its existing disk data; no second data copy was created.'
        } catch {
            Copy-Item -LiteralPath $sourceFile -Destination $staged -ErrorAction Stop
            Write-Output 'File sharing is unavailable between these folders. Copied the small mod DLL instead.'
        }
        if (Test-Path -LiteralPath $Destination) { Remove-Item -LiteralPath $Destination -Force }
        Move-Item -LiteralPath $staged -Destination $Destination
    } finally {
        if (Test-Path -LiteralPath $staged) { Remove-Item -LiteralPath $staged -Force }
    }
}
