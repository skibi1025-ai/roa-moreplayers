# 0.1.5 checks

Five roster selections used two sets of eight Workshop fighters: A, B, A, B, A. Private committed memory at each settled roster is recorded in MEMORY-PROFILE.json. The measured peak fell from 3,117,211,648 to 1,502,089,216 bytes (51.8%). Final roster A fell from 2,733,338,624 to 967,462,912 bytes. These measurements describe this fixture, rather than every possible Workshop lineup.

The comparison completed with eight simulated human controller inputs. UI matrices matched native matrix_build output, and the cached matrix returned to one owning reference. A separate eight-Workshop results fixture verified image-cache reuse, deleted all nine owned images, and returned to character selection. PNG-budget checks covered normal, oversized, zero-sized, malformed, and missing files. A shared-character fixture kept player 5 selected while player 1 switched fighters, verified that the shared head remained valid, then completed an eight-human match.

A 3,600-frame baseline match stayed near 1.05 GiB after loading, while repeated roster changes caused the larger growth. Large content combinations can still exceed the 32-bit process limit.

# Validation checks

The native eight-player fixture reached results, captured the victory and statistics views, verified placements and victory audio, and returned to character selection. A ready-up fixture checked that one human confirmation waits for the other human, then continues after both confirm. The fixture retries controlled knockouts if a fighter is in its respawn state. These forced knockouts are excluded from the normal DLL.

Additional installer and Workshop checks are recorded in VALIDATION.json. These use isolated test folders and copied game files.


## 0.1.4 checks

Two eight-human simulations used independent controller inputs and eight distinct Workshop fighters. Duck and Bubbles swapped between slots 1 and 5 in the second run. All eight fighters retained their starting stocks through 540 frames and used loaded fighter sprites. The first run was visually checked.

Earlier results and installer checks are the 0.1.3 baseline.
