# Validation checks

The native eight-player fixture reached results, captured the victory and statistics views, verified placements and victory audio, and returned to character selection. A ready-up fixture checked that one human confirmation waits for the other human, then continues after both confirm. The fixture retries controlled knockouts if a fighter is in its respawn state. These forced knockouts are excluded from the normal DLL.

Additional installer and Workshop checks are recorded in VALIDATION.json. These use isolated test folders and copied game files.


## 0.1.4 checks

Two eight-human simulations used independent controller inputs and eight distinct Workshop fighters. Duck and Bubbles swapped between slots 1 and 5 in the second run. All eight fighters retained their starting stocks through 540 frames and used loaded fighter sprites. The first run was visually checked.

Earlier results and installer checks are the 0.1.3 baseline.
