# Fix It In Post — modular tabletop kit

Generated with `tools/blender/make_kit.py`; validated by fresh-scene GLB re-import with `tools/blender/check_kit.py`. Units: metres. Blender Z up, directional front -Y; GLBs use glTF Y up. All origins are (0,0,0), with local geometry positioned as noted.

Preview sheet: four columns, five rows, in table order. Neutral grey world, three area lights, orthographic three-quarter camera.

| File | Triangles | Bounding box X × Y × Z (m) | Slot index: name | Origin / facing |
|---|---:|---|---|---|
| floor_tile.glb | 108 | 0.960 × 0.960 × 0.100 | 0: Paint, 1: Trim | Tile centre at floor Z=0; symmetric, no required facing. |
| floor_tile_b.glb | 268 | 0.960 × 0.960 × 0.101 | 0: Paint, 1: Trim | Tile centre at floor Z=0; symmetric, no required facing. |
| wall_block.glb | 324 | 1.000 × 1.000 × 0.550 | 0: Paint, 1: Trim | Tile centre at floor Z=0; symmetric, no required facing. |
| flat_backdrop.glb | 758 | 1.000 × 0.520 × 1.350 | 0: Backdrop, 1: Wood | Tile origin; panel centred Y=+0.5, front -Y; rear timber brace extends to Y≈1.0. |
| gate_block.glb | 540 | 0.860 × 0.860 × 0.820 | 0: Paint, 1: Accent, 2: Trim | Tile centre at floor Z=0; symmetric, no required facing. |
| gate_frame.glb | 592 | 1.100 × 1.100 × 0.120 | 0: Metal | Tile origin; 0.90 m clear opening, 0.10 m corner posts outside opening. |
| plate_base.glb | 160 | 0.800 × 0.800 × 0.030 | 0: Trim | Tile centre at floor Z=0; symmetric, no required facing. |
| plate_top.glb | 108 | 0.660 × 0.660 × 0.050 | 0: Accent | Own bottom centre at origin. |
| lever_base.glb | 540 | 0.340 × 0.440 × 0.140 | 0: Trim, 1: Metal | Tile origin; console centre X=-0.28; front -Y. |
| lever_arm.glb | 764 | 0.160 × 0.160 × 0.600 | 0: Metal, 1: Accent | Pivot at rod bottom; +Z upright. Rod 0.52 m, ball centre at Z=0.52. |
| laser_frame.glb | 896 | 0.920 × 0.920 × 0.050 | 0: Trim, 1: Metal, 2: Glow | Tile centre at floor Z=0; symmetric, no required facing. |
| mark_tape.glb | 120 | 0.593 × 0.593 × 0.014 | 0: Accent | Tile centre at floor Z=0; symmetric, no required facing. |
| pedestal.glb | 432 | 0.340 × 0.340 × 0.330 | 0: Paint, 1: Trim, 2: Accent | Tile origin; 0.30 m plinth plus 0.03 m cushion. |
| gem.glb | 76 | 0.300 × 0.285 × 0.260 | 0: Glow | Bounding-box centre at origin; deliberate flat facets. |
| reel_gold.glb | 1148 | 0.440 × 0.078 × 0.440 | 0: Metal | Centre at origin; axis Y, front -Y. Five through cut-outs. |
| alert_mark.glb | 216 | 0.130 × 0.090 × 0.450 | 0: Glow | Bottom centre at origin; front -Y. |
| star.glb | 260 | 0.220 × 0.090 × 0.200 | 0: Glow | Bounding-box centre at origin; front -Y. |
| sight_tile.glb | 60 | 0.900 × 0.900 × 0.000 | 0: Glow, 1: Accent | Tile centre at floor Z=0; symmetric, no required facing. |
| plinth.glb | 324 | 1.000 × 1.000 × 0.300 | 0: Wood | Tile centre at floor Z=0; symmetric, no required facing. |
| desk.glb | 108 | 1.000 × 1.000 × 0.100 | 0: Wood | Tile origin; top Z=0. Packed box UV islands; repeat texture sampler for tiling. |

Interpretations: gate-frame collar and posts sit outside the 0.90 m opening; lever ball sits at the end of the 0.52 m rod; pedestal cushion sits above the 0.30 m plinth. Flat backdrop includes its rear brace. Thin hardware bevels are proportionally smaller than body bevels. Each mesh is UV unwrapped; Backdrop alone uses a dedicated upright full-square UV face.

Validation: PASS (20/20)
