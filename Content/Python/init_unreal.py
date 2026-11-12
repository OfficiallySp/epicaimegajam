# Runs when the editor starts (Python plugin). Registers the jam's extra MCP toolset; never fatal.
import unreal

try:
    import mj_devtools
    mj_devtools.register()
    unreal.log("mj_devtools: MJDevTools toolset registered")
except Exception as exc:  # the editor must still open if the tooling breaks
    unreal.log_warning(f"mj_devtools: not registered: {exc}")
