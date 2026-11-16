# 0.1.1 validation

## Stock handling

The previous build reproduced a single knockout consuming three stocks in slots 5-8: the fighters remained outside the blast zone after the first stock decrement. The patched build places extra fighters at a valid stage spawn point during the surviving-stock respawn state.

Six native-roster rounds passed with three controlled knockouts per opposing player, checking 3 -> 2 -> 1 -> 0 stock transitions, results, and return to character selection. All 18 native fighters and Sandbag appeared in slots 5-8.

## Random selection

A stage-selection fixture exercised the native Random resolution event for eight active slots. All eight selections resolved to native fighters, completed gameplay, reached results, and returned to character selection. CPU flags were restored before gameplay.

## Workshop fighters

An eight-player match loaded eight distinct installed Workshop fighters:

| Slot | Fighter | Workshop ID |
| --- | --- | --- |
| 1 | Bubbles | 3014312006 |
| 2 | Sakamoto | 2085116778 |
| 3 | Infernape | 2242316629 |
| 4 | Grunkle Stan | 3690442640 |
| 5 | Duck | 3037252388 |
| 6 | Team Rocket | 2139756191 |
| 7 | Link | 2058525660 |
| 8 | Baller | 3162607695 |

The fixture exercised repeated stock knockouts, results, and return to character selection. The audio engine reported the victory fanfare playing during results.

## Memory

The executable already has Large Address Aware enabled. A proposed Workshop cache change did not demonstrate asset reuse and was excluded. Eight Workshop fighters loaded successfully in the selected fixture; large asset combinations can still exhaust the 32-bit process address space. Loading and fatal-error logs now record available virtual memory.

## Package

The release contains only the mod, source, and documentation. The installer verifies the supported executable and original DLL. The updater verifies package and installation hashes and retains a backup of the previous mod files. Saves are preserved. Regression builds use automated fixture definitions; the release DLL uses EXPERIMENTAL_MATCH only.
