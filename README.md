# Rivals Eight Local

Experimental Windows mod for the original Rivals of Aether 2.1.9.1. Adds up to eight players in Local Versus.

This is an experimental preview. There may be bugs. Replays are not currently a feature of this mod.

## Features

- Eight character-selection slots with fighter names.
- Local controller and keyboard input.
- Eight-player HUD with percentages, character heads, and stocks.
- Results screen with all participants, placements, stocks, and KOs.
- Support for installed Steam Workshop fighters, subject to individual compatibility.

## Requirements

Windows, Steam, and an installed copy of the supported game version are required. The installer checks the exact executable and original DLL. Other game versions are rejected.

Local Versus only. Online play is not supported. A Steam Workshop subscription does not install this native mod; the separate release ZIP is required.

## Installation

1. Download `RivalsEightLocal-0.1.0-preview.zip` from this repository's Releases page and extract it.
2. Close all Rivals windows. Keep Steam running.
3. Open PowerShell in the extracted directory and run this command with your game installation path:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Install.ps1 -GamePath "C:\Program Files (x86)\Steam\steamapps\common\Rivals of Aether"
```

The installer creates a separate copy at `%LOCALAPPDATA%\RivalsEightLocal\Game`. It does not modify the Steam installation. The execution-policy option applies only to this script process.

4. Run `Launch.cmd` in the separate game directory.
5. Select Local > Versus. Join with the keyboard and/or controllers.

For another destination, add `-Destination "D:\Games\RivalsEightLocal"`. The destination must be new and separate from the Steam game directory. Use `-ValidateOnly` to check files without installing.

Do not distribute the game directory created by the installer. The release ZIP contains only the mod, source code, and documentation.

## Save files

The separate copy shares the original game's AppData save directory. Saves are not isolated. `Launch.cmd` backs up save/settings files before each launch to `%LOCALAPPDATA%\RivalsEightLocal\SaveBackups`. Workshop asset directories are excluded.

To restore a backup, close both copies and copy the selected backup files into `%LOCALAPPDATA%\RivalsofAether`. Do not run both game copies at the same time.

## Troubleshooting and removal

Errors are recorded in `slot-probe.log` beside the copied executable. Include reproduction steps, game version, affected slots, and Workshop item IDs in bug reports. Review logs for personal paths before sharing them.

To remove the mod, close it and delete only its separate installation directory. Shared saves and save backups remain in AppData. The Steam installation remains unchanged.

## Build

Use Zig 0.13.0:

```powershell
.\source\Build.ps1 -Zig "C:\path\to\zig.exe"
```

The release DLL enables `EXPERIMENTAL_MATCH`. Automated test and screenshot definitions are disabled. Source files contain no comments.

## License

The original mod code uses the included MIT license. Game and Workshop character assets are not included. This project is not affiliated with Aether Studios or Valve.
