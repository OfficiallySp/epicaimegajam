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


def run(force=False):
    out = [create_solid(force), create_glow(force), create_stage_map()]
    print("; ".join(out))
    return out
