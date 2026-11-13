"""Deterministic tabletop kit. Run from any directory with Blender 5.2."""
import bpy, math, json
from pathlib import Path
from mathutils import Vector

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'assets/src/kit'
PREVIEW = OUT / 'preview'
PREVIEW.mkdir(parents=True, exist_ok=True)
COLORS = {'Paint': (.38,.57,.60,1), 'Trim': (.075,.115,.14,1),
          'Accent': (.95,.32,.085,1), 'Glow': (1,.69,.20,1),
          'Metal': (.57,.49,.32,1), 'Wood': (.36,.19,.085,1),
          'Backdrop': (.65,.76,.72,1)}
SLOTS = [['Paint','Trim'],['Paint','Trim'],['Paint','Trim'],['Backdrop','Wood'],
 ['Paint','Accent','Trim'],['Metal'],['Trim'],['Accent'],['Trim','Metal'],
 ['Metal','Accent'],['Trim','Metal','Glow'],['Accent'],['Paint','Trim','Accent'],
 ['Glow'],['Metal'],['Glow'],['Glow'],['Glow','Accent'],['Wood'],['Wood']]
NAMES = 'floor_tile floor_tile_b wall_block flat_backdrop gate_block gate_frame plate_base plate_top lever_base lever_arm laser_frame mark_tape pedestal gem reel_gold alert_mark star sight_tile plinth desk'.split()
NOTES = {
 'flat_backdrop': 'Tile centre at floor; image faces -Y; panel centre Y=+0.5. Timber and rear brace extend beyond tile.',
 'lever_base': 'Tile centre at floor; console centre X=-0.28; front -Y.',
 'lever_arm': 'Pivot at rod bottom (0,0,0); upright +Z; place at console pivot in game.',
 'plate_top': 'Own bottom centre at (0,0,0).',
 'gem': 'Geometric bounding-box centre at (0,0,0); facets intentionally flat.',
 'reel_gold': 'Centre at (0,0,0); reel axis Y; face toward -Y.',
 'star': 'Centre at (0,0,0); face toward -Y.',
 'alert_mark': 'Bottom centre at (0,0,0); face toward -Y.',
 'plinth': 'Tile centre at floor; top Z=-0.10; bottom Z=-0.40.',
 'desk': 'Tile centre at floor; top Z=0; box UVs use metres, suitable for tiled wood.',
 'floor_tile': 'Tile centre at floor; top Z=0; bottom Z=-0.10.',
 'floor_tile_b': 'Tile centre at floor; top Z=0; bottom Z=-0.10.',
 'wall_block': 'Tile centre at floor; bottom Z=-0.10.',
 'sight_tile': 'Tile centre at floor; surface Z=0.01.'}

def material(name):
    m=bpy.data.materials.get(name) or bpy.data.materials.new(name)
    m.use_nodes=True
    p=m.node_tree.nodes.get('Principled BSDF')
    p.inputs['Base Color'].default_value=COLORS[name]
    p.inputs['Roughness'].default_value=.47
    p.inputs['Metallic'].default_value=.55 if name=='Metal' else 0
    if name=='Glow':
        p.inputs['Emission Color'].default_value=COLORS[name]
        p.inputs['Emission Strength'].default_value=.25
    return m

def finish(o,mat,bevel=0,smooth=False):
    o.data.materials.append(material(mat))
    bpy.context.view_layer.objects.active=o
    bpy.ops.object.transform_apply(location=False,rotation=True,scale=True)
    if bevel:
        mod=o.modifiers.new('Soft painted edges','BEVEL'); mod.width=bevel; mod.segments=2
        bpy.ops.object.modifier_apply(modifier=mod.name)
    for p in o.data.polygons: p.use_smooth=smooth or bool(bevel)
    if smooth or bevel:
        mod=o.modifiers.new('Weighted corner normals','WEIGHTED_NORMAL'); mod.keep_sharp=True; mod.weight=50
        bpy.ops.object.modifier_apply(modifier=mod.name)
    return o

def box(loc,dim,mat,bevel=.018):
    bpy.ops.mesh.primitive_cube_add(size=1,location=loc); o=bpy.context.object; o.dimensions=dim
    return finish(o,mat,bevel)

def cyl(loc,r,depth,mat,axis='Z',verts=20,bevel=.005):
    bpy.ops.mesh.primitive_cylinder_add(vertices=verts,radius=r,depth=depth,location=loc)
    o=bpy.context.object
    if axis=='Y': o.rotation_euler.x=math.pi/2
    return finish(o,mat,bevel,True)

def mesh(name,verts,faces,mat):
    d=bpy.data.meshes.new(name); d.from_pydata(verts,[],faces); d.update()
    o=bpy.data.objects.new(name,d); bpy.context.collection.objects.link(o)
    return finish(o,mat)

def ring(size,width,z,height,mat,radius=.045):
    # Continuous rounded-square annulus; no overlapping corner bars.
    loops=[]
    for zz in (z,z+height):
        for half,r in ((size/2,radius),(size/2-width,max(.012,radius-width))):
            pts=[]
            for cx,cy,a in ((half-r,half-r,0),(-half+r,half-r,90),(-half+r,-half+r,180),(half-r,-half+r,270)):
                for j in range(5):
                    t=math.radians(a+j*90/4); pts.append((cx+r*math.cos(t),cy+r*math.sin(t),zz))
            loops.append(pts)
    v=sum(loops,[]); f=[]; n=20
    for i in range(n):
        j=(i+1)%n
        f.extend([(i,j,40+j,40+i),(20+j,20+i,60+i,60+j),(40+i,40+j,60+j,60+i),(j,i,20+i,20+j)])
    return mesh('rounded ring',v,f,mat)

def beam(a,b,width,mat):
    a,b=Vector(a),Vector(b); o=box((a+b)/2,(width,width,(b-a).length),mat,.008)
    o.rotation_euler=(b-a).to_track_quat('Z','Y').to_euler(); return o

def build(name):
    if name in ('floor_tile','floor_tile_b'):
        o=box((0,0,-.05),(.96,.96,.10),'Trim')
        o.data.materials.append(material('Paint'))
        for p in o.data.polygons:
            if p.normal.z>.7: p.material_index=1
        if name.endswith('_b'): ring(.83,.009,-.001,.0015,'Trim',.025)
    elif name=='wall_block':
        box((0,0,-.065),(1,1,.07),'Paint',.02)
        box((0,0,-.015),(.94,.94,.03),'Trim',.01)
        box((0,0,.225),(1,1,.45),'Paint',.025)
    elif name=='flat_backdrop':
        box((0,.5,.675),(1,.03,1.35),'Wood',.004)
        mesh('image face',[(-.5,.4845,0),(.5,.4845,0),(.5,.4845,1.35),(-.5,.4845,1.35)],[(0,1,2,3)],'Backdrop')
        for x in (-.455,.455): box((x,.545,.675),(.07,.06,1.35),'Wood',.008)
        for z in (.055,.675,1.295): box((0,.545,z),(.84,.06,.07),'Wood',.008)
        beam((0,.59,1.02),(0,.98,.035),.06,'Wood')
    elif name=='gate_block':
        box((0,0,.38),(.82,.82,.76),'Trim',.018)
        for lo,hi in ((0,.24),(.258,.49),(.508,.76)):
            box((0,0,(lo+hi)/2),(.86,.86,hi-lo),'Paint',.018)
        box((0,0,.79),(.86,.86,.06),'Accent',.018)
    elif name=='gate_frame':
        ring(1.10,.10,0,.02,'Metal',.04)
        for x in (-.5,.5):
            for y in (-.5,.5): box((x,y,.06),(.1,.1,.12),'Metal',.012)
    elif name=='plate_base': ring(.8,.055,0,.03,'Trim',.065)
    elif name=='plate_top': box((0,0,.025),(.66,.66,.05),'Accent',.024)
    elif name=='lever_base':
        box((-.28,0,.06),(.34,.44,.12),'Trim',.018)
        for x in (-.395,-.165): box((x,0,.13),(.11,.38,.02),'Metal',.008)
        for y in (-.17,.17): box((-.28,y,.13),(.12,.04,.02),'Metal',.006)
    elif name=='lever_arm':
        cyl((0,0,.26),.026,.52,'Metal')
        bpy.ops.mesh.primitive_uv_sphere_add(segments=24,ring_count=12,radius=.08,location=(0,0,.52))
        finish(bpy.context.object,'Accent',smooth=True)
    elif name=='laser_frame':
        ring(.92,.035,0,.03,'Trim',.05)
        for x,y in ((-.33,-.2),(-.11,.07),(.11,-.07),(.33,.2)):
            cyl((x,y,.023),.03,.046,'Metal',verts=12,bevel=.003)
            cyl((x,y,.048),.022,.004,'Glow',verts=12,bevel=0)
    elif name=='mark_tape':
        for angle in (-math.pi/4,math.pi/4):
            pts=[(-.36,-.065),(.35,-.065),(.36,-.025),(.347,.065),(-.35,.065),(-.36,.01)]
            v=[(x*math.cos(angle)-y*math.sin(angle),x*math.sin(angle)+y*math.cos(angle),z) for z in (0,.012) for x,y in pts]
            f=[tuple(range(5,-1,-1)),tuple(range(6,12))]+[(i,(i+1)%6,(i+1)%6+6,i+6) for i in range(6)]
            o=mesh('tape',v,f,'Accent')
            if angle>0:
                for vertex in o.data.vertices: vertex.co.z+=.002
            mod=o.modifiers.new('Tape soft edges','BEVEL'); mod.width=.001; mod.segments=1
            bpy.context.view_layer.objects.active=o; bpy.ops.object.modifier_apply(modifier=mod.name)
    elif name=='pedestal':
        box((0,0,.025),(.34,.34,.05),'Trim',.014)
        box((0,0,.145),(.29,.29,.23),'Paint',.018)
        box((0,0,.275),(.34,.34,.05),'Trim',.014)
        box((0,0,.315),(.24,.24,.03),'Accent',.014)
    elif name=='gem':
        v=[]
        for r,z in ((.077,.13),(.15,.045),(.15,.02),(.012,-.13)):
            v += [(r*math.cos(i*math.tau/10),r*math.sin(i*math.tau/10),z) for i in range(10)]
        f=[tuple(range(10))]
        for k in range(3):
            for i in range(10):
                a=k*10+i;b=k*10+(i+1)%10;c=b+10;d=a+10
                f += [(a,d,c),(a,c,b)]
        f.append(tuple(range(39,29,-1))); mesh('diamond',v,f,'Glow')
    elif name=='reel_gold':
        o=cyl((0,0,0),.22,.07,'Metal','Y',48,0)
        for i in range(5):
            a=i*math.tau/5+math.pi/2
            hole=cyl((.125*math.cos(a),0,.125*math.sin(a)),.057,.15,'Metal','Y',16,0)
            bpy.context.view_layer.objects.active=o
            mod=o.modifiers.new('Round lightening hole','BOOLEAN'); mod.operation='DIFFERENCE'; mod.object=hole
            bpy.ops.object.modifier_apply(modifier=mod.name); bpy.data.objects.remove(hole,do_unlink=True)
        mod=o.modifiers.new('Rim highlights','BEVEL'); mod.width=.003; mod.segments=1
        bpy.context.view_layer.objects.active=o; bpy.ops.object.modifier_apply(modifier=mod.name)
        for p in o.data.polygons: p.use_smooth=abs(p.normal.y)<.99
        mod=o.modifiers.new('Reel weighted normals','WEIGHTED_NORMAL');mod.keep_sharp=True
        bpy.ops.object.modifier_apply(modifier=mod.name)
        cyl((0,0,0),.045,.078,'Metal','Y',20,0)
    elif name=='alert_mark':
        box((0,0,.06),(.12,.09,.12),'Glow',.025)
        box((0,0,.31),(.13,.09,.28),'Glow',.025)
    elif name=='star':
        v=[(0,-.045,0),(0,.045,0)]
        for i in range(10):
            a=math.pi/2+i*math.tau/10; r=.115 if i%2==0 else .055
            v.append((r*math.cos(a),0,r*math.sin(a)))
        f=[]
        for i in range(10): f += [(0,2+i,2+(i+1)%10),(1,2+(i+1)%10,2+i)]
        o=mesh('puffy star',v,f,'Glow')
        mod=o.modifiers.new('Puffed seams','BEVEL'); mod.width=.008; mod.segments=2
        bpy.context.view_layer.objects.active=o; bpy.ops.object.modifier_apply(modifier=mod.name)
        for p in o.data.polygons:p.use_smooth=True
        # Centre its actual silhouette, including bevel.
        for axis in range(3):
            mid=(max(v.co[axis] for v in o.data.vertices)+min(v.co[axis] for v in o.data.vertices))/2
            for v in o.data.vertices:v.co[axis]-=mid
            extent=max(v.co[axis] for v in o.data.vertices)-min(v.co[axis] for v in o.data.vertices)
            for v in o.data.vertices:v.co[axis]*=(.22,.09,.20)[axis]/extent
    elif name=='sight_tile':
        o=ring(.9,.04,.01,0,'Glow',.07)
        # Remove redundant bottom and zero-height walls: a genuinely flat quad surface.
        import bmesh
        bm=bmesh.new();bm.from_mesh(o.data)
        bmesh.ops.delete(bm,geom=[f for f in bm.faces if f.index%4!=2],context='FACES');bm.to_mesh(o.data);bm.free()
        pts=[]
        for cx,cy,a in ((.38,.38,0),(-.38,.38,90),(-.38,-.38,180),(.38,-.38,270)):
            for j in range(5):
                t=math.radians(a+j*22.5);pts.append((cx+.03*math.cos(t),cy+.03*math.sin(t),.01))
        # Fan provides a robust convex triangulation.
        mesh('inner',pts+[(0,0,.01)],[(20,i,(i+1)%20) for i in range(20)],'Accent')
    elif name=='plinth':
        box((0,0,-.365),(1,1,.07),'Wood',.02)
        box((0,0,-.25),(.96,.96,.20),'Wood',.02)
        box((0,0,-.125),(1,1,.05),'Wood',.018)
    elif name=='desk': box((0,0,-.05),(1,1,.1),'Wood',.025)

def combine(name,slots):
    bpy.ops.object.select_all(action='DESELECT')
    obs=[o for o in bpy.context.scene.objects if o.type=='MESH']
    for o in obs:o.select_set(True)
    bpy.context.view_layer.objects.active=obs[0];bpy.ops.object.join();o=bpy.context.object
    bpy.context.scene.cursor.location=(0,0,0);bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
    bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
    assignments=[o.data.materials[p.material_index].name for p in o.data.polygons]
    o.data.materials.clear()
    for s in slots:o.data.materials.append(material(s))
    for p,s in zip(o.data.polygons,assignments):p.material_index=slots.index(s)
    o.name=name;o.data.name=name
    bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.uv.smart_project(angle_limit=math.radians(66),island_margin=.018)
    bpy.ops.object.mode_set(mode='OBJECT')
    uv=o.data.uv_layers.active.data
    for p in o.data.polygons:
        if slots[p.material_index]=='Backdrop':
            for li in p.loop_indices:
                co=o.data.vertices[o.data.loops[li].vertex_index].co;uv[li].uv=(co.x+.5,co.z/1.35)
    return o

def render(o,name):
    scene=bpy.context.scene
    scene.render.engine='CYCLES';scene.cycles.samples=24
    scene.render.resolution_x=512;scene.render.resolution_y=512;scene.render.resolution_percentage=100
    scene.world.color=(.22,.22,.22)
    scene.view_settings.view_transform='AgX'
    coords=[o.matrix_world@Vector(c) for c in o.bound_box];centre=sum(coords,Vector())/8
    size=max(o.dimensions);distance=max(size, .3)*2.7
    bpy.ops.object.camera_add(location=centre+Vector((1.4,-2,1.9)).normalized()*distance)
    cam=bpy.context.object;cam.rotation_euler=(centre-cam.location).to_track_quat('-Z','Y').to_euler()
    cam.data.type='ORTHO';cam.data.ortho_scale=max(size,.3)*1.65;scene.camera=cam
    for offset,power,colour in (((-3,-4,6),450,(1,.83,.65)),((4,-1,3),240,(.70,.82,1)),((1,4,5),550,(1,1,1))):
        bpy.ops.object.light_add(type='AREA',location=centre+Vector(offset)*max(size,.3))
        l=bpy.context.object;l.data.energy=power*max(size,.3)**2;l.data.shape='DISK';l.data.size=3*max(size,.3);l.data.color=colour
        l.rotation_euler=(centre-l.location).to_track_quat('-Z','Y').to_euler()
    scene.render.filepath=str(PREVIEW/(name+'.png'));bpy.ops.render.render(write_still=True)

def main():
    for name,slots in zip(NAMES,SLOTS):
        bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
        build(name);o=combine(name,slots)
        bpy.ops.object.select_all(action='DESELECT');o.select_set(True);bpy.context.view_layer.objects.active=o
        bpy.ops.export_scene.gltf(filepath=str(OUT/(name+'.glb')),export_format='GLB',use_selection=True,export_yup=True,export_animations=False,export_cameras=False,export_lights=False)
        render(o,name)
    # Blender-only contact sheet, no Pillow dependency. Row order matches NAMES.
    sheet=bpy.data.images.new('Contact sheet',width=2048,height=2560,alpha=False)
    from array import array
    pixels=array('f',[0])*(2048*2560*4)
    for i,name in enumerate(NAMES):
        im=bpy.data.images.load(str(PREVIEW/(name+'.png')),check_existing=False)
        src=array('f',[0])*(512*512*4);im.pixels.foreach_get(src)
        x=(i%4)*512;y=(4-i//4)*512
        for row in range(512):
            start=((y+row)*2048+x)*4;pixels[start:start+2048]=src[row*2048:(row+1)*2048]
        bpy.data.images.remove(im)
    sheet.pixels.foreach_set(pixels);sheet.filepath_raw=str(PREVIEW/'_sheet.png');sheet.file_format='PNG';sheet.save()
    print('KIT GENERATED: 20 GLBs and 21 previews')

if __name__=='__main__':main()
