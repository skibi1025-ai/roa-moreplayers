# Rivals Eight Local

Adds up to eight players to Local Versus in the original Rivals of Aether. Requires Windows and the Steam version of Rivals of Aether 2.1.9.1.

There may be bugs. Replays are not currently a feature of this mod.

## Install

1. Open [Releases](https://github.com/skibi1025-ai/roa-moreplayers/releases) and download the newest **RivalsEightLocal** ZIP under **Assets**. Choose the mod ZIP, not Source code.
2. Right-click the ZIP, select **Extract All**, and open the extracted folder.
3. Close Rivals and keep Steam running.
4. Double-click **Setup.cmd** and wait for **Setup complete**.
5. Double-click **Launch.cmd** in the same folder, then choose **Local > Versus**.

Setup normally finds the game automatically. If it asks for a folder, follow the instructions in the setup window. No commands need to be typed for a normal installation. PowerShell is used automatically.

## Update

Download the newest mod ZIP and extract it into a new folder. Close the game and double-click **Setup.cmd** in the new download. It updates your existing installation. Use **Launch.cmd** afterward. Saves are preserved and previous mod files are backed up.

If an older installation is in a custom location and setup cannot find it, double-click **Update.cmd** and paste the installed game folder address when asked.

## Game location

Setup creates a separate game copy at `%LOCALAPPDATA%\RivalsEightLocal\Game` by default. Use **Launch.cmd** in the extracted download folder to play; there is no need to find the installed folder. The original Steam files are unchanged.

To open the installed folder manually, press **Windows+R**, paste the location above, and press **Enter**. Its **Launch.cmd** also works. Keep all files from the download together.

## Features

- Eight character-selection slots with fighter names.
- Keyboard and controller input.
- Eight-player HUD with percentages, character heads, and stocks.
- Results screen with placements, stocks, KOs, and victory music.
- Installed Workshop fighters, subject to individual compatibility.

Local Versus only. Online play is not supported. A Workshop subscription alone does not install the mod.

## Help

The ZIP includes **READ-ME.txt** with setup, update, save, and removal information. If setup fails, share its exact error message. Game errors are recorded in `slot-probe.log` in the installed folder.

Large Workshop character combinations can run out of memory. Try fewer large characters or a simpler stage, then restart.

The copied game shares the original save folder. The launcher backs up saves and settings before each launch to `%LOCALAPPDATA%\RivalsEightLocal\SaveBackups`.

## Source and license

Build the source with Zig 0.13.0 using `source/Build.ps1`. The mod uses the included MIT license. Game and Workshop assets are not included. Do not distribute the copied game folder.

This project is not affiliated with Aether Studios or Valve.

## Repository folders

- `installer/`: setup, launcher, and mod files.
- `source/`: gameplay source and build script.
- `docs/`: release notes and validation records.

For installation, download the ZIP from Releases. The release ZIP layout is unchanged.
