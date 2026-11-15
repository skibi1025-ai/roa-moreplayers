# Validation record

Date: November 15, 2026. Platform: Windows, original Rivals of Aether 2.1.9.1, one physical controller plus keyboard.

## Latest CPU selection update

The CPU selection changes were checked without launching the game or controlling the desktop, as requested. Compilation, native patch signatures, emulation of the actual token-scan and pickup predicates, CPU-level validation, and token-layout checks passed. The CPU-level array now includes slots 5-8, slot 5 is no longer skipped by token pickup, and the scan reaches slot 8. All eight token positions have separate pickup areas. CPU cursor ownership and menu-transition cleanup were updated in source.

In-game confirmation of difficulty buttons, changing CPU characters, re-grabbing placed CPU tokens, and Workshop selection is still pending. The earlier gameplay and rendering checks below describe prior builds; they do not establish runtime verification of this CPU update.

The later screenshot session exposed and corrected a separate child-control placement problem: native footer buttons were still positioned for full-height cards. The creation hook now places these controls inside their own compressed card, above its name caption. A new in-game capture shows difficulty bars for CPU slots 3-8 with their separate values 4-9, two human cards, and readable fighter names. That capture build also completed an eight-player match with distinct placements and returned from results to selection. This confirms rendering and the automated match path; it does not confirm human CPU-character re-selection or clicking difficulty arrows.

## Passed

- Character-card caption rendering was checked across all eight occupied slots, including Charlie Kirk and Sans. Captions display selected fighter names in a separate lower row clear of the card menu icons. The name text uses uniform scaling and fits within its available width. Unoccupied cards retain their join prompt.

- The user confirmed controller switching into extra character-selection slots works.
- The user confirmed Workshop Playtest is visible and the portrait aspect ratio is corrected.
- The user confirmed the corrected keyboard/controller assignments work independently. The fix uses zero-based controller indexes and one-based player indexes.
- Runtime inspection showed the controller assigned to P1 and the keyboard to P8 without duplicate controller assignments.
- Automated match: all eight native fighters received at least 358 updates, damage occurred, controlled final-stock knockouts completed, and the results room was reached.
- The user confirmed Ready to Fight and stage selection accept input with Workshop characters in slots 6 and 8.
- Automated sparse match: only slots 6 and 8 were active; each received at least 359 updates, the match reached results, and P6 Start returned to character selection without an error.
- The stage-input handler accepted movement and confirmation separately from slots 6 and 8.
- The latest six automated eight-player matches on the standard hazard-free tree stage passed with the new victory screen. All 18 base-game fighters and Sandbag (internal character IDs 2-20) were exercised in slots 5-8. Each player received at least 300 updates; extra slots retained three stocks until the planned knockouts. Every match reached results and returned to character selection. These are short smoke tests, not exhaustive moveset or matchup tests.
- The user confirmed the previously reported crash when returning from a Workshop match no longer occurs.
- The results screen was captured and inspected after replacing its incomplete font with a readable standard font.
- The new eight-player victory screen passed a staggered knockout test: seven players were eliminated in order, the sole survivor was correctly declared the winner, all eight distinct placements were verified, and P6 Start returned to character selection. The captured screen was inspected for portrait proportions and all eight player entries.
- A sparse Workshop regression with only P6 and P8 active reached the new victory screen, correctly named Sans as the P8 winner, showed the individual Workshop heads, and returned to character selection. Each active fighter received 360 updates. Unoccupied slots no longer count as survivors.
- Automated Workshop matches loaded Charlie Kirk (item 3799054154) in P6 and Sans (Legacy, item 3824607103) in P8. Both characters rendered on-stage and their own HUD heads appeared. Separate tests completed each character's final-stock knockout and returned from results to character selection.
- A Workshop regression started P8 with stale zero-stock/off state; new-match preparation restored its three stocks and active state. An invalid unselected attacker was injected into Charlie Kirk's knockout; the new guard cleared it and the match completed without an access violation. Charlie Kirk's own script intentionally changes its stock count to one.
- Both HUD layouts were captured in-game. The user chose all eight entries along the bottom. Original four-player HUD cards are suppressed.
- The normal candidate compiles with eight-player gameplay enabled, without forced-CPU, preview-freeze, or automated-match definitions.
- Installer validation, an actual separate-copy installation, source/destination overlap rejection, existing-destination rejection, and unsupported-executable rejection passed.
- Original Steam executable and original DLL hashes remained unchanged.
- The runtime executable guard accepted the tested copied executable and rejected an unrelated executable in its isolated test.
- The launcher rejected a concurrent game instance.
- The installed launcher launched the normal build and created timestamped backups of the shared save directory.
- Shipped PowerShell files parse successfully and have no comment tokens. Shipped C files and headers have no comments.
- The distributable excludes game executables, original game DLLs, game data, and Workshop assets.

## Pending release checks

- Verify CPU difficulty controls in all eight slots, CPU/human/off transitions, independent re-selection of stacked native/Workshop tokens, and selecting Workshop fighters for CPUs. No test game was launched for this update.

- User confirmation of the latest normal build with Charlie Kirk and Sans (Legacy), including initial visibility, independent human input during the match, and leaving the stage. The preceding manual build still lost P8 and crashed during a knockout, despite native-roster tests passing. The new build adds fresh match state, knockout validation, persistent keyboard assignment, and runtime diagnostics.

## Limits of this preview

Eight simultaneous human inputs have not been tested. One controller does not establish compatibility with eight physical or Parsec controllers. Workshop characters can contain four-player assumptions and require individual testing. Teams, replays, every stage, alternate game modes, and future game versions are not validated. The copied game shares the original game's save directory.

A stage with hazards produced an unset-argument error during an automated menu-flow test. Hazardous stages are not certified by these results. The current Abyss hazard-free substitution also needs a dedicated runtime test.

This record does not certify a stable or universal eight-player release. Do not remove the experimental label when sharing the preview.
