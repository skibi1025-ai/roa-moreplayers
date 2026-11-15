param([string]$Zig = 'zig')
$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    & $Zig cc -target x86-windows-gnu -shared -O1 -DBG_ADDR=0x41b2ea0 -DCUR_ADDR=0x424be60 -DLOCAL_ADDR=0x419bae0 -DCREATE_ADDR=0x4248b90 -DEXPERIMENTAL_MATCH slot_probe.c -o catch_error_mini.dll -lkernel32 -ladvapi32 -luser32
    if ($LASTEXITCODE -ne 0) { throw 'Compilation failed.' }
} finally { Pop-Location }
