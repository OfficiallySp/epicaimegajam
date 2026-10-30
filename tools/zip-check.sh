#!/usr/bin/env bash
# Zip a packaged build as TeamName_GameName.zip and check it against the jam rules (Mac/Linux twin of zip-check.ps1).
# Usage:
#   tools/zip-check.sh                          zips dist/Win64/Windows (or dist/Mac/Mac/*.app) -> dist/TeamName_GameName.zip
#   tools/zip-check.sh <build-dir|Game.app>     zip a specific folder or app bundle
#   tools/zip-check.sh --check-only <file.zip>  only check an existing zip (for example one downloaded from itch)
set -euo pipefail
. "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/common.sh"
jam_load_env

MAX_BYTES="${JAM_MAX_ZIP_BYTES:-1000000000}"
check_only=0; src=""; zip=""
while [ $# -gt 0 ]; do
  case "$1" in
    --check-only) check_only=1 ;;
    --zip) zip="$2"; shift ;;
    -h|--help) sed -n '2,6p' "$0"; exit 0 ;;
    *) if [ "$check_only" -eq 1 ] && [ -z "$zip" ]; then zip="$1"; else src="$1"; fi ;;
  esac
  shift
done

problems=(); warnings=()
if [ "$check_only" -eq 0 ]; then
  base="$(jam_zip_base)"
  zip="${zip:-$REPO_ROOT/dist/$base.zip}"
  if [ -z "$src" ]; then
    if [ -d "$REPO_ROOT/dist/Win64/Windows" ]; then
      src="$REPO_ROOT/dist/Win64/Windows"
    else
      src="$(ls -d "$REPO_ROOT"/dist/Mac/Mac/*.app 2>/dev/null | head -1 || true)"
    fi
  fi
  if [ -z "$src" ] || [ ! -e "$src" ]; then
    fail "No build found. Run tools/package.sh (Mac) or copy a Win64 build to dist/Win64/Windows."; exit 1
  fi
  mkdir -p "$(dirname "$zip")"; rm -f "$zip"
  step "Zipping $src -> $zip"
  case "$src" in
    *.app) ditto -c -k --sequesterRsrc --keepParent "$src" "$zip" ;;
    *) ( cd "$src" && zip -r -X -q "$zip" . -x '*.DS_Store' -x '*.pdb' ) ;;
  esac
fi

if [ -z "${zip:-}" ] || [ ! -f "$zip" ]; then fail "Zip not found: ${zip:-<none>}"; exit 1; fi
step "Checking $zip"
name="$(basename "$zip")"
size="$(file_size "$zip")"
[[ "$name" =~ ^[A-Za-z0-9]+_[A-Za-z0-9]+\.zip$ ]] || problems+=("Name '$name' is not TeamName_GameName.zip (letters and digits, one underscore)")
if [ "$size" -ge "$MAX_BYTES" ]; then
  problems+=("Size $(human_size "$size") is not under the limit of $MAX_BYTES bytes ($(human_size "$MAX_BYTES"))")
elif [ "$size" -ge $((MAX_BYTES * 9 / 10)) ]; then
  warnings+=("Size $(human_size "$size") is within 10% of the limit; trim maps, textures or packs")
fi
entries="$(unzip -Z1 "$zip")"
count="$(printf '%s\n' "$entries" | grep -c . || true)"
launcher="$(printf '%s\n' "$entries" | grep -E '^[^/]+\.exe$|^[^/]+\.app/$' || true)"
[ -n "$launcher" ] || problems+=("No .exe (or .app) at the zip root. Judges expect: unzip, double-click.")
printf '%s\n' "$entries" | grep -qE '^Engine/|\.app/Contents/' || warnings+=("No Engine/ folder at the zip root. Is this a UE packaged build?")
if printf '%s\n' "$entries" | grep -qE '\.pdb$'; then warnings+=("The zip contains .pdb files: wasted space"); fi
echo "    entries: $count; zip $(human_size "$size"); launcher: $(printf '%s' "$launcher" | tr '\n' ' ')"

for w in ${warnings[@]+"${warnings[@]}"}; do warn "$w"; done
if [ "${#problems[@]}" -gt 0 ]; then
  for p in "${problems[@]}"; do fail "$p"; done
  exit 1
fi
printf '\033[32mOK: %s (%s) passes the jam checks. Extract it into a NEW folder and run it, then tools/upload.sh\033[0m\n' "$name" "$(human_size "$size")"
