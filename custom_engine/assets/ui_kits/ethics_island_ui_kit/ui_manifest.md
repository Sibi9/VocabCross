# UI & UX Asset Manifest: Ethics Island UI Kit
**Theme:** Floating Governance & Audit Saga Map (`ethics_island_ui_kit`)  
**Target Platform:** Mobile / Casual Saga Map / Level Progression Games  
**Installed Skills Used:** `create-game-assets` & `game-ui-ux`  
**Resolution Standard:** High-DPI / Retina Ready (RGBA 32-bit PNG with Alpha Transparency)

---

## 1. Dynamic Theme & Visual Audit (الهوية الموضوعية والبصمة البصرية)

### Theme Concept
- **Setting:** Floating mystical island in a pastel sky ("ETHICS ISLAND").
- **Hero Landmark:** Classical marble temple topped with a giant golden Scale of Justice.
- **Narrative Elements:** An auditor/detective examining compliance documents, an ice cube treasure vault, and a ringing golden herald bell.
- **UI Mood:** Playful, vibrant, clean 3D casual aesthetics with candy colors, smooth bevels, and tactile rounded surfaces.

### Color Palette Bible (لوحة الألوان الدقيقة)

| Role | Color Name | HEX Code | RGB | Usage |
| :--- | :--- | :--- | :--- | :--- |
| **Sky Lilac** | Pastel Lilac | `#FFD4E5` | `(255, 212, 229)` | Top gradient of sky background |
| **Sky Mint** | Mint Turquoise | `#5CE1E6` | `(92, 225, 230)` | Lower gradient of sky background |
| **Island Turf**| Spring Grass Green | `#48C23C` | `(72, 194, 60)` | Floating island grass & path curbs |
| **Justice Gold**| Solar Amber Gold | `#FFD034` | `(255, 208, 52)` | Scales of Justice, golden bell, reward stars |
| **Gold Bevel** | Roasted Bronze | `#D97706` | `(217, 119, 6)` | 3D bevel of gold items & star borders |
| **Active Node** | Electric Cyan Blue | `#38BDF8` | `(56, 189, 248)` | Current playable level node (Node 1) |
| **Active Bevel**| Deep Sky Blue | `#0284C7` | `(2, 132, 199)` | 3D bottom bevel of active level node |
| **Stone Node** | Slate Stone Gray | `#CBD5E1` | `(203, 213, 225)`| Locked level nodes (Nodes 2-8) |
| **Stone Bevel**| Charcoal Slate | `#64748B` | `(100, 116, 139)`| 3D bottom bevel of stone nodes |
| **Signpost Wood**| Honey Oak Wood | `#F4A261` | `(244, 162, 97)` | Wooden signpost plank and support posts |
| **Bubble Pink** | Coral Pink | `#FF6584` | `(255, 101, 132)`| 3D bubble letter E, candy alphabet |
| **Bubble Orange**| Sunset Orange | `#FF8C38` | `(255, 140, 56)` | 3D bubble letter T, candy alphabet |
| **Bubble Purple**| Royal Amethyst | `#9D4EDD` | `(157, 78, 221)` | 3D bubble letter D, candy alphabet |

---

## 2. Exhaustive Asset Directory & Specifications (قائمة الأصول المولدة)

### Folder: `ui_elements/`

| File Name | Dimensions | Format | Description & Usage |
| :--- | :---: | :---: | :--- |
| `background_clean.png` | 1024x1792 | PNG | Seamless vertical 9:16 background of pastel gradient sky and 3D clouds. Free of islands and UI. |
| `ethics_island_full_map.png` | 571x1024 | RGB PNG | Complete assembled reference saga level map. |
| `signpost_ethics_island.png` | 250x140 | RGBA PNG | Wooden signpost with 3D colorful bubble letters "ETHICS ISLAND". |
| `signpost_blank.png` | 250x140 | RGBA PNG | Clean blank wooden signpost for custom level/world names. |
| `signpost_blank_3d.png` | 920x680 | RGBA PNG | High-resolution 3D blank wooden signpost on transparent background. |
| `scales_of_justice.png` | 210x195 | RGBA PNG | Golden Scales of Justice landmark isolated from game map. |
| `scales_of_justice_3d.png` | 890x890 | RGBA PNG | High-resolution 3D polished golden Scales of Justice. |
| `temple_courthouse.png` | 235x255 | RGBA PNG | Classical marble courthouse building with pillars and pediment. |
| `glass_vault_safe.png` | 170x195 | RGBA PNG | Translucent glowing glass cube safe with treasure chests and vault dial. |
| `auditor_detective.png` | 215x185 | RGBA PNG | Auditor detective mascot with magnifying glass and compliance papers. |
| `golden_bell.png` | 135x155 | RGBA PNG | Ringing golden herald bell on pedestal with sound waves. |

---

### Folder: `level_nodes/` (خانات وأزرار المراحل)

| File Name | Dimensions | Format | Details |
| :--- | :---: | :---: | :--- |
| `node_blank_active.png` | 256x256 | RGBA PNG | Active level node template with glowing cyan ring and 3D bevel. |
| `node_blank_stone.png` | 256x256 | RGBA PNG | Locked level node template with slate gray 3D bevel. |
| `node_blank_gold.png` | 256x256 | RGBA PNG | Completed level node template with radiant gold 3D bevel. |
| `icon_star_gold.png` | 128x128 | RGBA PNG | 3D golden reward star with faceted highlights. |
| `icon_star_empty.png` | 128x128 | RGBA PNG | 3D silver/gray empty star slot. |
| `icon_padlock.png` | 128x128 | RGBA PNG | 3D silver padlock indicating locked levels. |
| `level_node_1.png` | 256x256 | RGBA PNG | Level 1 active node with "1" and 2-gold-star rating pedestal. |
| `level_node_2.png` to `8.png`| 256x256 | RGBA PNG | Numbered locked nodes with 3 empty star slots and lock badge. |
| `level_nodes_sheet.png` | 1024x512 | RGBA PNG | Sprite sheet of all level nodes (4x2 grid) for engine slicing. |

---

### Folder: `alphabet_tiles/` (Full 26 A-Z English Alphabet)

| File Name | Dimensions | Format | Details |
| :--- | :---: | :---: | :--- |
| `tile_blank.png` | 256x256 | RGBA PNG | Master circular 3D jelly/stone tile template. |
| `tile_A.png` to `tile_Z.png` | 256x256 | RGBA PNG | **All 26 English Alphabet letters** rendered individually in Ethics Island candy 3D bubble style. |
| `alphabet_A_to_Z_sheet.png` | 1792x1024 | RGBA PNG | Master sprite sheet in 7x4 grid (26 letters + blank tile). |
| `alphabet_A_to_Z_sheet.json`| - | JSON | TexturePacker / Unity Sprite Atlas slicing metadata coordinates. |

---

## 3. UI/UX Architecture & Engine Integration (تطبيق مهارة game-ui-ux)

### Responsive Layout & Anchors
- **Design Reference Resolution:** `1080 x 1920` (9:16 portrait) with `CanvasScaler` / `Stretch Mode: canvas_items`.
- **Top Safe Area:** Inset HUD icons (back button, settings) by 44px on iOS / Android punch-hole displays.
- **Bottom Safe Area:** Inset the `signpost_ethics_island.png` by 34px from screen bottom.
- **Scroll View:** Place the level nodes on a `ScrollRect` (Unity) or `ScrollContainer` (Godot) with smooth elastic bounce.

### Event-Driven Level Progression
- **No per-frame polling:** Nodes listen to `level_unlocked(int level_id)` and `stars_awarded(int level_id, int count)`.
- **Focus Navigation:** Gamepad D-pad Up/Down navigates between sequential level nodes (`node_1` -> `node_2` -> ...).

