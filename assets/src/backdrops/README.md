# OpenAI backdrop and key art provenance

Generated 2026-11-14 using the official Meshy MCP via tools/mcp/meshy-call.mjs, explicitly selecting OpenAI `gpt-image-2` for every image.

Balance before: **761** credits. Balance after: **641** credits. Spent: **120** (12 text images at 9 each; one reference image at 12). Hard cap: 200. No paid regenerations. The R3_4 request was rejected once because the queue was full; the task list was checked before resubmission. No create timeouts occurred.

## Outputs and assembly

The three earlier procedural atlases were replaced to put OpenAI-generated art visibly on the game sets. Four distinct portrait generations per reel are preserved in raw/. `python tools/assemble_backdrops.py` center-crops/resizes to 512x640, applies shared palette edge strips (16 px opaque, feathering to 48 px), and repaints shared top/base bands. Museum wainscot begins at y=460; alley base at 570; lair red line at 596. It writes RGB 2048x640 atlases and contact.png (three atlases stacked, then three shuffled eight-panel rows). All 48 directed panel-edge pairings are pixel-identical. No engine reimport was performed.

The image-to-image reference was devlog/img/12-coverage-closeup.png. Key art is assets/src/keyart/keyart_raw.png, with a 630x500 crop at devlog/img/cover-630x500-ai.png. Title added deterministically using Content/Fonts/BebasNeue-Regular.ttf. Existing cover preserved.

Visual review: strong readable motifs; no readable generated lettering spotted. Brick courses and riveted plate seams do not align within the feathered zones; shared vertical strips read as theatre-flat joins rather than continuous masonry. Museum generated rail remnants can appear just above the shared wainscot. The blank-faced portrait is intentionally generic. Final in-game scale and lighting need parent-session review after reimport.

## Licence (checked 2026-11-14)

[Meshy Terms of Use, sections 2.4 and 3.2](https://www.meshy.ai/terms-of-use) define Customer Output to include images and state: ?such customers on a paid Meshy plan own their Customer Output.? This is qualified by applicable law. Free-plan outputs instead require Meshy attribution under CC BY 4.0. No community publication was requested. Third-party input rights remain the customer's responsibility.

[Meshy's image-generator page](https://www.meshy.ai/features/ai-image-generator) states: ?Paid plan users retain full commercial usage rights to all images generated.? These are contractual rights, not a guarantee of copyright protection. The supplied reference is the project's own in-game frame.

## Exact prompts and task IDs

### R1_1

Tool: `meshy_text_to_image`; model: `gpt-image-2`; credits: 9; task: `01a1db0e-0011-764d-bae8-ba6d18438cea`; aspect ratio: 2:3.

One straight-on orthographic painted theatre wall flat, edge-to-edge portrait wall texture for a miniature 1960s spy caper diorama. Bold simple silhouettes, hand-painted gouache and handmade brush texture, readable at tiny scale. No perspective, floor, ceiling, people, letters, numbers, logos or watermark. Keep all motifs inside central 70 percent width. Teal striped museum wallpaper (#1e6874), antique gold picture rail near top, brown wood wainscot bottom 28 percent. Invented gold-framed sunset mountain landscape and blank brass plaque.

### R1_2

Tool: `meshy_text_to_image`; model: `gpt-image-2`; credits: 9; task: `01a1db0e-34e6-73d2-947c-5ecc6a8db08d`; aspect ratio: 2:3.

One straight-on orthographic painted theatre wall flat, edge-to-edge portrait wall texture for a miniature 1960s spy caper diorama. Bold simple silhouettes, hand-painted gouache and handmade brush texture, readable at tiny scale. No perspective, floor, ceiling, people, letters, numbers, logos or watermark. Keep all motifs inside central 70 percent width. Teal striped museum wallpaper (#1e6874), antique gold picture rail near top, brown wood wainscot bottom 28 percent. Single elegant brass wall sconce with cream triangular shade.

### R1_3

Tool: `meshy_text_to_image`; model: `gpt-image-2`; credits: 9; task: `01a1db0e-39b5-7214-9f4a-25d6abb27a1f`; aspect ratio: 2:3.

One straight-on orthographic painted theatre wall flat, edge-to-edge portrait wall texture for a miniature 1960s spy caper diorama. Bold simple silhouettes, hand-painted gouache and handmade brush texture, readable at tiny scale. No perspective, floor, ceiling, people, letters, numbers, logos or watermark. Keep all motifs inside central 70 percent width. Teal striped museum wallpaper (#1e6874), antique gold picture rail near top, brown wood wainscot bottom 28 percent. Gold-framed generic anonymous painted aristocrat portrait and blank brass plaque.

### R1_4

Tool: `meshy_text_to_image`; model: `gpt-image-2`; credits: 9; task: `01a1db0e-3e89-7070-9ecd-fa5450c75bb0`; aspect ratio: 2:3.

One straight-on orthographic painted theatre wall flat, edge-to-edge portrait wall texture for a miniature 1960s spy caper diorama. Bold simple silhouettes, hand-painted gouache and handmade brush texture, readable at tiny scale. No perspective, floor, ceiling, people, letters, numbers, logos or watermark. Keep all motifs inside central 70 percent width. Teal striped museum wallpaper (#1e6874), antique gold picture rail near top, brown wood wainscot bottom 28 percent. Small cream exhibition placard with abstract geometric pictograms, no text.

### R2_1

Tool: `meshy_text_to_image`; model: `gpt-image-2`; credits: 9; task: `01a1db0e-4365-729a-bb32-3f701e6eb19c`; aspect ratio: 2:3.

One straight-on orthographic painted theatre wall flat, edge-to-edge portrait wall texture for a miniature 1960s spy caper diorama. Bold simple silhouettes, hand-painted gouache and handmade brush texture, readable at tiny scale. No perspective, floor, ceiling, people, letters, numbers, logos or watermark. Keep all motifs inside central 70 percent width. Rust red brick alley wall (#7e3e30), dark plum mortar and grime along bottom 11 percent. Torn cream and red graphic poster with abstract circle, no lettering.

### R2_2

Tool: `meshy_text_to_image`; model: `gpt-image-2`; credits: 9; task: `01a1db0e-4856-7100-8742-41d96f9e18b6`; aspect ratio: 2:3.

One straight-on orthographic painted theatre wall flat, edge-to-edge portrait wall texture for a miniature 1960s spy caper diorama. Bold simple silhouettes, hand-painted gouache and handmade brush texture, readable at tiny scale. No perspective, floor, ceiling, people, letters, numbers, logos or watermark. Keep all motifs inside central 70 percent width. Rust red brick alley wall (#7e3e30), dark plum mortar and grime along bottom 11 percent. Bare weathered brick with subtle damp patches.

### R2_3

Tool: `meshy_text_to_image`; model: `gpt-image-2`; credits: 9; task: `01a1db0e-4d1e-77f9-88c8-78f9ca0a7365`; aspect ratio: 2:3.

One straight-on orthographic painted theatre wall flat, edge-to-edge portrait wall texture for a miniature 1960s spy caper diorama. Bold simple silhouettes, hand-painted gouache and handmade brush texture, readable at tiny scale. No perspective, floor, ceiling, people, letters, numbers, logos or watermark. Keep all motifs inside central 70 percent width. Rust red brick alley wall (#7e3e30), dark plum mortar and grime along bottom 11 percent. Grey metal pipe and bold red valve wheel.

### R2_4

Tool: `meshy_text_to_image`; model: `gpt-image-2`; credits: 9; task: `01a1db0e-51d0-76ee-830d-75132abbeceb`; aspect ratio: 2:3.

One straight-on orthographic painted theatre wall flat, edge-to-edge portrait wall texture for a miniature 1960s spy caper diorama. Bold simple silhouettes, hand-painted gouache and handmade brush texture, readable at tiny scale. No perspective, floor, ceiling, people, letters, numbers, logos or watermark. Keep all motifs inside central 70 percent width. Rust red brick alley wall (#7e3e30), dark plum mortar and grime along bottom 11 percent. Small barred window glowing amber with grey sill.

### R3_1

Tool: `meshy_text_to_image`; model: `gpt-image-2`; credits: 9; task: `01a1db0e-5682-77d0-962b-b9d8358c69ff`; aspect ratio: 2:3.

One straight-on orthographic painted theatre wall flat, edge-to-edge portrait wall texture for a miniature 1960s spy caper diorama. Bold simple silhouettes, hand-painted gouache and handmade brush texture, readable at tiny scale. No perspective, floor, ceiling, people, letters, numbers, logos or watermark. Keep all motifs inside central 70 percent width. Burgundy riveted steel villain lair wall (#4a1e24), black seams, red horizontal floor line near bottom. Invented red hollow triangle crest inside black circular medallion.

### R3_2

Tool: `meshy_text_to_image`; model: `gpt-image-2`; credits: 9; task: `01a1db0e-5b32-72c1-a679-ff8ee2e7a6d5`; aspect ratio: 2:3.

One straight-on orthographic painted theatre wall flat, edge-to-edge portrait wall texture for a miniature 1960s spy caper diorama. Bold simple silhouettes, hand-painted gouache and handmade brush texture, readable at tiny scale. No perspective, floor, ceiling, people, letters, numbers, logos or watermark. Keep all motifs inside central 70 percent width. Burgundy riveted steel villain lair wall (#4a1e24), black seams, red horizontal floor line near bottom. Central yellow and charcoal diagonal hazard stripe panel.

### R3_3

Tool: `meshy_text_to_image`; model: `gpt-image-2`; credits: 9; task: `01a1db0e-6026-7066-aca5-b946f9f25095`; aspect ratio: 2:3.

One straight-on orthographic painted theatre wall flat, edge-to-edge portrait wall texture for a miniature 1960s spy caper diorama. Bold simple silhouettes, hand-painted gouache and handmade brush texture, readable at tiny scale. No perspective, floor, ceiling, people, letters, numbers, logos or watermark. Keep all motifs inside central 70 percent width. Burgundy riveted steel villain lair wall (#4a1e24), black seams, red horizontal floor line near bottom. Retro black monitor with green waveform and three coloured indicator lights, no text.

### R3_4

Tool: `meshy_text_to_image`; model: `gpt-image-2`; credits: 9; task: `01a1db0e-f291-7364-a166-2796d37f000b`; aspect ratio: 2:3.

One straight-on orthographic painted theatre wall flat, edge-to-edge portrait wall texture for a miniature 1960s spy caper diorama. Bold simple silhouettes, hand-painted gouache and handmade brush texture, readable at tiny scale. No perspective, floor, ceiling, people, letters, numbers, logos or watermark. Keep all motifs inside central 70 percent width. Burgundy riveted steel villain lair wall (#4a1e24), black seams, red horizontal floor line near bottom. Bold dark steel ventilation grille.

### keyart

Tool: `meshy_image_to_image`; model: `gpt-image-2`; credits: 12; task: `01a1db0e-f77c-73b0-83a6-bac967f149df`; aspect ratio: 3:2.

Restyle this exact game scene as an expressive hand-painted 1960s spy-film poster, gouache brushwork, bold cinematic silhouettes, warm cream, teal, burgundy and gold. Preserve the scene layout, miniature set, props and characters of the reference. No text, letters, numbers, logos or watermark. Not photorealistic.
