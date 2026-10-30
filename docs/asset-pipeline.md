# AI asset pipeline tests: Sun 8 Nov, about 4 h

Goal: one asset through each tool into UE at the right scale with materials, with the steps and gotchas written here so
jam-day imports are mechanical. Record the time per asset; that number decides how many assets the jam can afford.
Every import also gets a row in `docs/asset-declarations.md`.

## Test matrix (fill in)
| # | Tool | Asset to make | Export format | Import route | Scale and orientation OK | Materials OK | Minutes | License note (verify on the tool's terms page) |
|---|---|---|---|---|---|---|---|---|
| 1 | Tripo | rigged humanoid character | FBX or GLB | UE import; retarget to the UE5 mannequin or use with Mixamo animations | | | | commercial rights on which plan? |
| 2 | Meshy | prop (e.g. a crate or lantern) | FBX/GLB + textures | drag-drop import | | | | free tier output license (attribution?) |
| 3 | Hyper3D / Rodin | hero prop | GLB/FBX, high poly | Blender decimate (or Nanite) then import | | | | |
| 4 | ElevenLabs | 5 SFX + 1 voice line | WAV/MP3 | ffmpeg to 48 kHz WAV, import | n/a | n/a | | commercial use needs a paid plan? |
| 5 | Suno or Udio | 60–90 s loopable music | WAV/MP3 | ffmpeg to WAV, Looping on the Sound Wave | n/a | n/a | | commercial use terms |
| 6 | Image model | 8 UI icons + 1 seamless texture | PNG | import; UI textures: no mips, UI group | n/a | | | |
| 7 | Blockade Labs Skybox | one skybox | equirect PNG/HDR | HDRI Backdrop plugin or sky sphere | n/a | | | |
| 8 | Mixamo (if Tripo animations fall short) | 3 animations on the Tripo rig | FBX | import onto the character skeleton | | | | |

## Import rules for UE (expected; correct after testing)
- Units: UE is centimetres, Z up. Check scale against the mannequin (`SKM_Manny`, 180 cm) placed next to the import.
- glTF/GLB imports through Interchange (File > Import or drag-drop); materials arrive as instances of the glTF material functions.
- FBX from Blender: apply transforms; export with Scale 1.0 and Apply Scalings "FBX Units Scale"; Forward −Y, Up Z;
  skeletal: Only Deform Bones, Add Leaf Bones off. If a rig comes in 100× too big or small, set Blender's unit scale to 0.01 and re-export.
- Collision: Static Mesh editor > Collision > auto convex, or the import option "Generate Missing Collision"; boxes are fine for props.
- Nanite: enable on dense AI meshes (Static Mesh editor > Nanite Settings) instead of decimating; not for skeletal meshes or masked/translucent materials.
- Textures: normal maps need Compression "Normalmap" and sRGB off; roughness/metallic/AO packed maps sRGB off; 2K max for a jam; UI textures in the UI texture group with mips off.
- Audio: 16-bit or 24-bit PCM WAV at 48 kHz, mono for SFX, stereo for music. `ffmpeg -i in.mp3 -ar 48000 -ac 2 -c:a pcm_s16le out.wav`. Set Looping on music; sound classes SFX/Music/UI from day one.
- Skybox: HDR as compression HDR with sRGB off; `HDRIBackdrop` plugin for a quick sky, or a sky sphere with an unlit material. Blockade output is 360° equirectangular.
- Retargeting: UE 5.4+ Animation > Retarget Animations auto-generates IK rigs for humanoids; check hands and feet, then batch-duplicate the animations onto the new skeleton.
- Batch imports go through a Python script in `Content/Python/` (`unreal.AssetImportTask`, `unreal.AssetToolsHelpers`), so 20 files cost the same effort as one.

## Fab shortlist (add to the library before 12 Nov; check each license, UE-only licenses are fine)
| Pack | Use | License checked | Added |
|---|---|---|---|
| Megascans surfaces and a few props | environment materials | | |
| Game Animation Sample | locomotion for a humanoid (migrate only the needed animations; the project is large) | | |
| Stack O Bot | robot character, simple world kit | | |
| Paragon characters (free) | hero characters with animations | | |
| Infinity Blade assets (free) | weapons, props, effects | | |
| This month's free Fab assets | whatever fits | | |
| A Niagara VFX pack | impacts, pickups | | |
| A UI/sound starter pack | menu sounds, HUD frames | | |

Size caution: everything imported is cooked only if referenced by a shipped map or listed in "additional directories to
cook", but large packs still bloat the repo. Import to `Content/External/<Pack>/`, migrate what is used, delete the rest
before the final package if the zip is near 1 GB.
