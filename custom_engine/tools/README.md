# Tools

These scripts document and reproduce the verification assets. They are not
required at runtime.

| Script | Purpose |
| :--- | :--- |
| `make_clean_backdrops.py` | Creates `assets/scenery/clean_*.png` by masking every baked HUD/node/star/lock/text and inpainting the masked areas. Uses the normalized comparison targets in `reference_targets/ref_*.jpg`. |
| `check_arabic_font.py` | Checks which Arabic presentation forms each bundled font provides. Cairo is missing 33; Tahoma/Segoe UI cover all 154. |
| `font_compare.py` | Renders Arabic samples with Cairo/Tahoma/Segoe UI for visual inspection. |
| `update_data.py` | Normalizes the vocabulary JSON (BRIBERY, ECLIPSE, GALAXY wheel mode, exact letter arrays). |
| `interactive_test.ps1` | End-to-end Windows input test: types BRIBERY in the real crossword, reveals the remaining crossing words, clicks the ECLIPSE keyboard, and opens the real wheel crossword from the map. Saves `verification_reports/interactive_*.png`. |

Run them from the project root with the bundled Python:

```powershell
py tools/make_clean_backdrops.py
py tools/check_arabic_font.py
```

The normalized comparison targets (`reference_targets/ref_*.jpg`) are included
so that `verify_and_compare.py` can run without the original user attachments.
