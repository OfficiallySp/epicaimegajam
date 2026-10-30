#!/usr/bin/env bash
# Mac fallback twin of package.ps1: package the project for macOS with UAT BuildCookRun (Development).
# Only used if the PC is unavailable. Needs UE 5.8.4 for Mac and Xcode. Output: dist/Mac/Mac/<Project>.app
# Usage: tools/package.sh [--clean] [--maps L_A+L_B] [--config Development|Shipping] [--engine "/Users/Shared/Epic Games/UE_5.8"]
set -euo pipefail
. "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/common.sh"
jam_load_env

engine="$(jam_setting UE_ROOT_MAC "/Users/Shared/Epic Games/UE_5.8")"
config=Development; maps=""; clean=0
while [ $# -gt 0 ]; do
  case "$1" in
    --clean) clean=1 ;;
    --maps) maps="$2"; shift ;;
    --config) config="$2"; shift ;;
    --engine) engine="$2"; shift ;;
    -h|--help) sed -n '2,4p' "$0"; exit 0 ;;
    *) fail "Unknown argument: $1"; exit 1 ;;
  esac
  shift
done

runuat="$engine/Engine/Build/BatchFiles/RunUAT.sh"
[ -x "$runuat" ] || { fail "RunUAT.sh not found at $runuat. Set UE_ROOT_MAC in tools/jam.env or pass --engine."; exit 1; }
project="$(ls "$REPO_ROOT"/*.uproject 2>/dev/null | head -1 || true)"
[ -n "$project" ] || { fail "No .uproject in $REPO_ROOT (the repo root must be the Unreal project root)."; exit 1; }
name="$(basename "$project" .uproject)"
out="$REPO_ROOT/dist/Mac"; logdir="$REPO_ROOT/dist/logs"
if [ "$clean" -eq 1 ] && [ -d "$out" ]; then step "Removing $out"; rm -rf "$out"; fi
mkdir -p "$out" "$logdir"
log="$logdir/package-mac-$(date +%Y%m%d-%H%M%S).log"

args=(BuildCookRun "-project=$project" "-target=$name" -platform=Mac "-clientconfig=$config"
      -build -cook -stage -pak -iostore -compressed -package -archive "-archivedirectory=$out"
      -nop4 -utf8output -unattended -installed -nocompile -nocompileuat -nodebuginfo)
if [ -n "$maps" ]; then args+=("-map=$maps"); fi
if pgrep -x UnrealEditor >/dev/null 2>&1; then
  warn "UnrealEditor is running: the editor target will not be rebuilt (-skipbuildeditor). Close it after C++ changes."
  args+=(-skipbuildeditor -nocompileeditor)
fi

step "$name: $config Mac -> $out"
echo "    RunUAT ${args[*]}"
echo "    log: $log"
start="$(date +%s)"
set +e
"$runuat" "${args[@]}" 2>&1 | tee "$log"
code="${PIPESTATUS[0]}"
set -e
echo "    took $(( ($(date +%s) - start) / 60 )) min"
if [ "$code" -ne 0 ]; then fail "BuildCookRun failed (exit $code). Search for 'Error:' in $log"; exit "$code"; fi
app="$(ls -d "$out"/Mac/*.app 2>/dev/null | head -1 || true)"
step "Packaged: ${app:-$out} ($(du -sh "$out/Mac" 2>/dev/null | cut -f1))"
echo "If packaging fails on signing with Xcode 27: see docs/pc-bringup.md, section Fallback."
echo "Next: tools/zip-check.sh"
