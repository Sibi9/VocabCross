These three PNGs are SCENERY-ONLY backdrops used by the island maps.

They were generated from the visual references by tools/make_clean_backdrops.py:
- every HUD capsule, stage node, number, star, padlock, checkmark, "Cleared"
  label and wooden sign lettering was masked;
- OpenCV inpainting filled the masked areas;
- the remaining image is environment art only.

The C++ engine never loads assets/reference_ui/*. All interactive widgets
(nodes, stars, locks, HUD, signs, buttons, panels) are drawn procedurally in
include/ui_widgets.hpp, include/decor_art.hpp and include/screen_maps.hpp.

clean_ethics.png  -> Unit 1 Ethics Island scenery
clean_safety.png  -> Unit 2 Safety First Island scenery
clean_space.png   -> Unit 3 Space Island scenery

Unit 4 (Emotions Island) is drawn entirely in code by Art::CarnivalIsland().
