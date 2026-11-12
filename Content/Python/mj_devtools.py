"""Extra MCP toolset for the jam: run editor Python and console commands from the agent.

Editor-only development tooling. Nothing here is part of the packaged game.
"""
import contextlib
import io
import traceback

import unreal

import toolset_registry
from toolset_registry.registration import Registration


def _game_world():
    try:
        return unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
    except Exception:
        return None


def _editor_world():
    return unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()


@unreal.uclass()
class MJDevTools(unreal.ToolsetDefinition):
    """Jam dev tools: run Python in the editor and execute console commands (editor or PIE world)."""

    @toolset_registry.tool_call
    @staticmethod
    def run_python(code: str) -> str:
        """Executes Python source in the editor process with the `unreal` module available.

        Helpers in scope: `unreal`, `game_world()` (PIE world or None), `editor_world()`.
        Output printed with print() is captured. Assign to a variable named `result` to return a value.

        Args:
            code: Python source to execute.

        Returns:
            Captured stdout, then `RESULT: <repr>` when `result` was assigned, or the traceback on error.
        """
        buf = io.StringIO()
        env = {"unreal": unreal, "game_world": _game_world, "editor_world": _editor_world, "__name__": "__mj__"}
        try:
            with contextlib.redirect_stdout(buf):
                exec(compile(code, "<mj_devtools>", "exec"), env)
        except Exception:
            buf.write("\nEXCEPTION:\n" + traceback.format_exc())
        if "result" in env:
            buf.write("\nRESULT: " + repr(env["result"]))
        return buf.getvalue()

    @toolset_registry.tool_call
    @staticmethod
    def console(command: str) -> str:
        """Executes a console command in the PIE world when PIE is running, otherwise in the editor world.

        Args:
            command: The console command, e.g. `mj.Dump` or `stat fps`.

        Returns:
            Which world the command ran in. Read the Output Log for the command's own output.
        """
        world = _game_world()
        where = "PIE"
        if world is None:
            world = _editor_world()
            where = "editor"
        unreal.SystemLibrary.execute_console_command(world, command)
        return f"executed in {where} world: {command}"


_registration = Registration([MJDevTools])


def register():
    return _registration.register()
