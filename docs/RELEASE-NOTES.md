# 0.1.5 preview

- Workshop resource reuse and cleanup cover all eight slots.
- Reduced memory retained while changing Workshop lineups.
- Reused UI transform data instead of allocating a new matrix for each draw.
- Shared duplicate results images and released them when leaving results.
- Oversized results artwork uses the character head as a fallback.
- Extra slots use normal player initialization and Workshop drawing scripts.
- Added memory readings to slot-probe.log.

# 0.1.4 preview

- Fixed a white X replacing Workshop fighters in slot 5.
- Initialized missing control state for human players in slots 5â€“8.

# 0.1.3 preview

- The extracted folder contains Setup.cmd, Launch.cmd, READ-ME.txt, and files/.
- The installed mod DLL shares disk data with the downloaded file through a hard link when supported. A small file copy is used when linking is unavailable.
- Workshop directories are excluded from the base-game copy.
- The eight-player victory screen uses native RoA artwork, fonts, placement badges, a large winner portrait, and ranked rows.
- Z / A readies a human player. The screen continues once all human players are ready.
- X / Tab switches the statistics view. Start / Enter continues immediately.
- Results use native victory tracks followed by the native results loop, with the saved music volume.

There may be bugs. Replays are not currently a feature of this mod.
