"""One-off editor setup for Fix It In Post: base materials and the stage map. Idempotent; run from the editor.

    import mj_setup; mj_setup.run()
"""
import unreal

MAT_DIR = "/Game/MegaJam26/Materials"
MAP_PATH = "/Game/MegaJam26/Maps/L_Stage"

_mel = unreal.MaterialEditingLibrary
_eal = unreal.EditorAssetLibrary


def _new_material(name, force=False):
    path = f"{MAT_DIR}/{name}"
    if _eal.does_asset_exist(path):
        if not force:
            return None
        mat = _eal.load_asset(path)
        _mel.delete_all_material_expressions(mat)
        return mat
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    return tools.create_asset(name, MAT_DIR, unreal.Material, unreal.MaterialFactoryNew())


def _vec(mat, name, default, x, y):
    node = _mel.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, x, y)
    node.set_editor_property("parameter_name", name)
    node.set_editor_property("default_value", unreal.LinearColor(*default))
    return node


def _scalar(mat, name, default, x, y):
    node = _mel.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, x, y)
    node.set_editor_property("parameter_name", name)
    node.set_editor_property("default_value", default)
    return node


def create_solid(force=False):
    """Opaque lit colour: Color, Rough, Metal, Glow (emissive multiplier of Color)."""
    mat = _new_material("M_Solid", force)
    if mat is None:
        return "M_Solid exists"
    mat.set_editor_property("used_with_instanced_static_meshes", True)
    color = _vec(mat, "Color", (0.5, 0.5, 0.5, 1.0), -600, 0)
    rough = _scalar(mat, "Rough", 0.6, -600, 220)
    metal = _scalar(mat, "Metal", 0.0, -600, 320)
    glow = _scalar(mat, "Glow", 0.0, -600, 420)
    mul = _mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -300, 420)
    _mel.connect_material_expressions(color, "", mul, "A")
    _mel.connect_material_expressions(glow, "", mul, "B")
    _mel.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)
    _mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    _mel.connect_material_property(metal, "", unreal.MaterialProperty.MP_METALLIC)
    _mel.connect_material_property(mul, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    _mel.recompile_material(mat)
    _eal.save_loaded_asset(mat)
    return "M_Solid created"


def create_glow(force=False):
    """Unlit translucent glow: Color, Opacity, Boost."""
    mat = _new_material("M_Glow", force)
    if mat is None:
        return "M_Glow exists"
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property("two_sided", True)
    color = _vec(mat, "Color", (1.0, 0.2, 0.1, 1.0), -600, 0)
    boost = _scalar(mat, "Boost", 4.0, -600, 220)
    opacity = _scalar(mat, "Opacity", 0.5, -600, 320)
    mul = _mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -300, 100)
    _mel.connect_material_expressions(color, "", mul, "A")
    _mel.connect_material_expressions(boost, "", mul, "B")
    _mel.connect_material_property(mul, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    _mel.connect_material_property(opacity, "", unreal.MaterialProperty.MP_OPACITY)
    _mel.recompile_material(mat)
    _eal.save_loaded_asset(mat)
    return "M_Glow created"


def create_stage_map():
    """An empty level: the game builds its whole set, lights and camera in C++."""
    if _eal.does_asset_exist(MAP_PATH):
        return "L_Stage exists"
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    ok = les.new_level(MAP_PATH)
    les.save_current_level()
    return f"L_Stage created: {ok}"


def import_mesh(src, name, folder="/Game/MegaJam26/Characters", prefix="SM_"):
    """Import a GLB/FBX and leave its static mesh at <folder>/<prefix><name> (materials and textures in <folder>/<name>/)."""
    dest = f"{folder}/{name}"
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", src)
    task.set_editor_property("destination_path", dest)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    target = f"{folder}/{prefix}{name}"
    meshes = [p for p in _eal.list_assets(dest, recursive=True) if isinstance(_eal.load_asset(p), unreal.StaticMesh)]
    if not meshes:
        return f"{name}: no static mesh imported from {src}"
    if _eal.does_asset_exist(target):
        _eal.delete_asset(target)
    _eal.rename_asset(meshes[0].split(".")[0], target)
    _eal.save_directory(folder)
    mesh = _eal.load_asset(target)
    b = mesh.get_bounds()
    return f"{target}: tris={mesh.get_num_triangles(0)} extent=({b.box_extent.x:.0f},{b.box_extent.y:.0f},{b.box_extent.z:.0f}) meshes={len(meshes)}"


def _src(sub):
    """Absolute path of assets/src/<sub> in this project, wherever the project folder lives."""
    root = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
    return (root.rstrip("/") + "/assets/src/" + sub).replace("\\", "/")


def import_backdrops(src_dir=None):
    """Import the painted backdrop atlases (tools/make_backdrops.py) as /Game/MegaJam26/Environment/T_Backdrop_R<n>."""
    import glob
    import os
    src_dir = src_dir or _src("backdrops")
    tasks = []
    for f in sorted(glob.glob(src_dir + "/T_Backdrop_R*.png")):
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", f.replace("\\", "/"))
        task.set_editor_property("destination_path", "/Game/MegaJam26/Environment")
        task.set_editor_property("destination_name", os.path.splitext(os.path.basename(f))[0])
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("save", True)
        tasks.append(task)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
    return f"{len(tasks)} backdrop atlases imported"


def create_backdrop(force=False):
    """Lit backdrop for the flats: a 4-panel atlas, the panel picked by per-instance custom data 0. Params: Atlas, Tint, Lift."""
    mat = _new_material("M_Backdrop", force)
    if mat is None:
        return "M_Backdrop exists"
    mat.set_editor_property("used_with_instanced_static_meshes", True)
    uv = _mel.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -1100, 0)
    uv.set_editor_property("u_tiling", 0.25)
    uv.set_editor_property("v_tiling", 1.0)
    panel = _mel.create_material_expression(mat, unreal.MaterialExpressionPerInstanceCustomData, -1100, 200)
    panel.set_editor_property("data_index", 0)
    quarter = _mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -1100, 320)
    quarter.set_editor_property("r", 0.25)
    shift = _mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -900, 220)
    _mel.connect_material_expressions(panel, "", shift, "A")
    _mel.connect_material_expressions(quarter, "", shift, "B")
    zero = _mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -900, 360)
    zero.set_editor_property("r", 0.0)
    offset = _mel.create_material_expression(mat, unreal.MaterialExpressionAppendVector, -720, 260)
    _mel.connect_material_expressions(shift, "", offset, "A")
    _mel.connect_material_expressions(zero, "", offset, "B")
    uv2 = _mel.create_material_expression(mat, unreal.MaterialExpressionAdd, -540, 100)
    _mel.connect_material_expressions(uv, "", uv2, "A")
    _mel.connect_material_expressions(offset, "", uv2, "B")
    tex = _mel.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -340, 60)
    tex.set_editor_property("parameter_name", "Atlas")
    default_tex = _eal.load_asset("/Game/MegaJam26/Environment/T_Backdrop_R1")
    if default_tex:
        tex.set_editor_property("texture", default_tex)
    _mel.connect_material_expressions(uv2, "", tex, "UVs")
    tint = _vec(mat, "Tint", (1.0, 1.0, 1.0, 1.0), -340, 320)
    color = _mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -100, 120)
    _mel.connect_material_expressions(tex, "RGB", color, "A")
    _mel.connect_material_expressions(tint, "", color, "B")
    lift = _scalar(mat, "Lift", 0.12, -340, 520)
    glow = _mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -100, 360)
    _mel.connect_material_expressions(color, "", glow, "A")
    _mel.connect_material_expressions(lift, "", glow, "B")
    rough = _mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -100, 560)
    rough.set_editor_property("r", 0.88)
    _mel.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)
    _mel.connect_material_property(glow, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    _mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    _mel.recompile_material(mat)
    _eal.save_loaded_asset(mat)
    return "M_Backdrop created"


def import_surfaces(src_dir=None):
    """Import the tileable surface textures (tools/make_surfaces.py) as /Game/MegaJam26/Environment/T_Surface_<Name>."""
    import glob
    import os
    src_dir = src_dir or _src("surfaces")
    tasks = []
    for f in sorted(glob.glob(src_dir + "/T_Surface_*.png")):
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", f.replace("\\", "/"))
        task.set_editor_property("destination_path", "/Game/MegaJam26/Environment")
        task.set_editor_property("destination_name", os.path.splitext(os.path.basename(f))[0])
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("save", True)
        tasks.append(task)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
    return f"{len(tasks)} surface textures imported"


def create_surface(force=False):
    """Lit tiled texture: Tex, Tint, Gain, Strength (0 = plain Tint), Rough, Tiling (mesh UVs),
    UseWorld (1 = project world XY instead of mesh UVs) and WorldSize (world units per repeat)."""
    mat = _new_material("M_Surface", force)
    if mat is None:
        return "M_Surface exists"
    mat.set_editor_property("used_with_instanced_static_meshes", True)
    mat.set_editor_property("used_with_nanite", True)
    uv = _mel.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -1300, -100)
    tiling = _scalar(mat, "Tiling", 1.0, -1300, 0)
    uv_mesh = _mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -1100, -60)
    _mel.connect_material_expressions(uv, "", uv_mesh, "A")
    _mel.connect_material_expressions(tiling, "", uv_mesh, "B")
    wp = _mel.create_material_expression(mat, unreal.MaterialExpressionWorldPosition, -1300, 140)
    mask = _mel.create_material_expression(mat, unreal.MaterialExpressionComponentMask, -1100, 140)
    mask.set_editor_property("r", True)
    mask.set_editor_property("g", True)
    mask.set_editor_property("b", False)
    mask.set_editor_property("a", False)
    _mel.connect_material_expressions(wp, "", mask, "")
    world_size = _scalar(mat, "WorldSize", 400.0, -1300, 260)
    uv_world = _mel.create_material_expression(mat, unreal.MaterialExpressionDivide, -900, 160)
    _mel.connect_material_expressions(mask, "", uv_world, "A")
    _mel.connect_material_expressions(world_size, "", uv_world, "B")
    use_world = _scalar(mat, "UseWorld", 0.0, -900, 300)
    uv_pick = _mel.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, -700, 40)
    _mel.connect_material_expressions(uv_mesh, "", uv_pick, "A")
    _mel.connect_material_expressions(uv_world, "", uv_pick, "B")
    _mel.connect_material_expressions(use_world, "", uv_pick, "Alpha")
    tex = _mel.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -500, 0)
    tex.set_editor_property("parameter_name", "Tex")
    default_tex = _eal.load_asset("/Game/MegaJam26/Environment/T_Surface_Scenic")
    if default_tex:
        tex.set_editor_property("texture", default_tex)
    _mel.connect_material_expressions(uv_pick, "", tex, "UVs")
    gain = _scalar(mat, "Gain", 1.0, -500, 260)
    boosted = _mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -300, 60)
    _mel.connect_material_expressions(tex, "RGB", boosted, "A")
    _mel.connect_material_expressions(gain, "", boosted, "B")
    one = _mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -300, -60)
    one.set_editor_property("r", 1.0)
    strength = _scalar(mat, "Strength", 1.0, -300, 220)
    detail = _mel.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, -120, 40)
    _mel.connect_material_expressions(one, "", detail, "A")
    _mel.connect_material_expressions(boosted, "", detail, "B")
    _mel.connect_material_expressions(strength, "", detail, "Alpha")
    tint = _vec(mat, "Tint", (1.0, 1.0, 1.0, 1.0), -120, 220)
    color = _mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, 60, 80)
    _mel.connect_material_expressions(detail, "", color, "A")
    _mel.connect_material_expressions(tint, "", color, "B")
    rough = _scalar(mat, "Rough", 0.7, 60, 260)
    _mel.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)
    _mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    _mel.recompile_material(mat)
    _eal.save_loaded_asset(mat)
    return "M_Surface created"


def run(force=False):
    out = [create_solid(force), create_glow(force), create_stage_map()]
    print("; ".join(out))
    return out
