"""Independent re-import validation; also writes the measured asset manifest."""
import bpy, json
from pathlib import Path
from mathutils import Vector
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'assets/src/kit'
# Dimensions include explicitly requested attachments (cushion, ball, rear brace).
SPECS={
'floor_tile':((.96,.96,.10),['Paint','Trim'],(-.1,0)),
'floor_tile_b':((.96,.96,.10),['Paint','Trim'],(-.1,0)),
'wall_block':((1,1,.55),['Paint','Trim'],(-.1,.45)),
'flat_backdrop':((1,.54,1.35),['Backdrop','Wood'],(0,1.35)),
'gate_block':((.86,.86,.82),['Paint','Accent','Trim'],(0,.82)),
'gate_frame':((1.1,1.1,.12),['Metal'],(0,.12)),
'plate_base':((.8,.8,.03),['Trim'],(0,.03)),
'plate_top':((.66,.66,.05),['Accent'],(0,.05)),
'lever_base':((.34,.44,.14),['Trim','Metal'],(0,.14)),
'lever_arm':((.16,.16,.60),['Metal','Accent'],(0,.6)),
'laser_frame':((.92,.92,.05),['Trim','Metal','Glow'],(0,.05)),
'mark_tape':((.60,.60,.012),['Accent'],(0,.012)),
'pedestal':((.34,.34,.33),['Paint','Trim','Accent'],(0,.33)),
'gem':((.30,.2853,.26),['Glow'],(-.13,.13)),
'reel_gold':((.44,.07,.44),['Metal'],(-.22,.22)),
'alert_mark':((.13,.09,.45),['Glow'],(0,.45)),
'star':((.22,.09,.20),['Glow'],None),
'sight_tile':((.9,.9,0),['Glow','Accent'],(.01,.01)),
'plinth':((1,1,.30),['Wood'],(-.4,-.1)),
'desk':((1,1,.10),['Wood'],(-.1,0)),
}
NOTES={
'flat_backdrop':'Tile origin; panel centred Y=+0.5, front -Y; rear timber brace extends to Y≈1.0.',
'lever_base':'Tile origin; console centre X=-0.28; front -Y.',
'lever_arm':'Pivot at rod bottom; +Z upright. Rod 0.52 m, ball centre at Z=0.52.',
'plate_top':'Own bottom centre at origin.',
'gem':'Bounding-box centre at origin; deliberate flat facets.',
'reel_gold':'Centre at origin; axis Y, front -Y. Five through cut-outs.',
'star':'Bounding-box centre at origin; front -Y.',
'alert_mark':'Bottom centre at origin; front -Y.',
'pedestal':'Tile origin; 0.30 m plinth plus 0.03 m cushion.',
'gate_frame':'Tile origin; 0.90 m clear opening, 0.10 m corner posts outside opening.',
'desk':'Tile origin; top Z=0. Packed box UV islands; repeat texture sampler for tiling.',
}
rows=[]; errors=[]; report=[]
for name,(expected,slots,zrange) in SPECS.items():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.gltf(filepath=str(OUT/(name+'.glb')))
    objects=list(bpy.context.scene.objects);meshes=[o for o in objects if o.type=='MESH']
    def check(ok,msg):
        if not ok:errors.append(name+': '+msg)
    check(len(meshes)==1,'must contain exactly one mesh')
    if len(meshes)!=1:continue
    o=meshes[0];d=o.data;d.calc_loop_triangles();n=len(d.loop_triangles)
    check(all(abs(v-1)<1e-5 for v in o.scale),'non-unit scale')
    check(o.location.length<1e-5,'origin displaced')
    actual=[m.name for m in d.materials];check(actual==slots,'slots '+str(actual)+' != '+str(slots))
    check(50<=n<=1200,'triangle budget: '+str(n))
    coords=[o.matrix_world@v.co for v in d.vertices]
    lo=[min(v[i] for v in coords) for i in range(3)];hi=[max(v[i] for v in coords) for i in range(3)]
    dim=[hi[i]-lo[i] for i in range(3)]
    check(all(abs(a-b)<=.02001 for a,b in zip(dim,expected)),'dimensions '+str(dim)+' expected '+str(expected))
    if zrange:check(abs(lo[2]-zrange[0])<.02001 and abs(hi[2]-zrange[1])<.02001,'floor/height convention')
    if name in ('gem','reel_gold','star'):
        check(all(abs(hi[i]+lo[i])<.002 for i in range(3)),'not centred on origin')
    elif name!='flat_backdrop':
        target=-.28 if name=='lever_base' else 0
        check(abs((hi[0]+lo[0])/2-target)<.01 and abs(hi[1]+lo[1])<.02,'tile or pivot XY origin')
    check(bool(d.uv_layers),'missing UVs')
    check(not bpy.data.actions,'unexpected animation')
    check(len(objects)==1,'unexpected camera/light/extra object')
    if name=='flat_backdrop':
        check(abs(lo[1]-.4845)<.002,'panel front placement')
        uv=d.uv_layers.active.data
        for p in d.polygons:
            if actual[p.material_index]=='Backdrop':
                check(p.normal.y<-.99,'backdrop facing')
                for li in p.loop_indices:
                    v=d.vertices[d.loops[li].vertex_index].co
                    check((uv[li].uv-Vector((v.x+.5,v.z/1.35))).length<.001,'backdrop UV orientation')
    note=NOTES.get(name,'Tile centre at floor Z=0; symmetric, no required facing.')
    rows.append(f'| {name}.glb | {n} | '+ ' × '.join(f'{v:.3f}' for v in dim)+' | '+', '.join(f'{i}: {s}' for i,s in enumerate(actual))+' | '+note+' |')
    report.append(dict(name=name,triangles=n,bbox=dim,minimum=lo,maximum=hi,slots=actual))
text='# Fix It In Post — modular tabletop kit\n\nGenerated with `tools/blender/make_kit.py`; validated by fresh-scene GLB re-import with `tools/blender/check_kit.py`. Units: metres. Blender Z up, directional front -Y; GLBs use glTF Y up. All origins are (0,0,0), with local geometry positioned as noted.\n\nPreview sheet: four columns, five rows, in table order. Neutral grey world, three area lights, orthographic three-quarter camera.\n\n| File | Triangles | Bounding box X × Y × Z (m) | Slot index: name | Origin / facing |\n|---|---:|---|---|---|\n'+'\n'.join(rows)+'\n\nInterpretations: gate-frame collar and posts sit outside the 0.90 m opening; lever ball sits at the end of the 0.52 m rod; pedestal cushion sits above the 0.30 m plinth. Flat backdrop includes its rear brace. Thin hardware bevels are proportionally smaller than body bevels. Each mesh is UV unwrapped; Backdrop alone uses a dedicated upright full-square UV face.\n\nValidation: '+ ('PASS (20/20)' if not errors else 'FAIL\n'+'\n'.join(errors))+'\n'
(OUT/'README.md').write_text(text,encoding='utf-8')
(OUT/'measurements.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
print('\n'.join(rows))
assert not errors,'\n'.join(errors)
print('PASS: all 20 GLBs independently re-imported and checked.')
