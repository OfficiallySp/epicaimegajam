#!/usr/bin/env bash
# Push a build to itch.io with butler (Mac/Linux twin of upload.ps1).
# Usage: tools/upload.sh [zip-or-folder] [--channel win64-dev] [--target user/game] [--version v] [--dry-run]
# Defaults: dist/TeamName_GameName.zip, else dist/Win64/Windows, else dist/Mac/Mac. One-time setup: butler login
set -euo pipefail
. "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/common.sh"
jam_load_env
command -v butler >/dev/null 2>&1 || { fail "butler is not on PATH (see tools/README.md), then run: butler login"; exit 1; }

target="$(jam_setting ITCH_TARGET)"
channel="$(jam_setting ITCH_CHANNEL win64-dev)"
version=""; src=""; extra=()
while [ $# -gt 0 ]; do
  case "$1" in
    --channel) channel="$2"; shift ;;
    --target) target="$2"; shift ;;
    --version) version="$2"; shift ;;
    --dry-run) extra+=(--dry-run) ;;
    -h|--help) sed -n '2,4p' "$0"; exit 0 ;;
    *) src="$1" ;;
  esac
  shift
done
[[ "$target" =~ ^[a-z0-9-]+/[a-z0-9-]+$ ]] || { fail "ITCH_TARGET must be user/game-slug (tools/jam.env). Got '${target:-<empty>}'"; exit 1; }

if [ -z "$src" ]; then
  base="$(jam_zip_base 2>/dev/null || true)"
  if [ -n "$base" ] && [ -f "$REPO_ROOT/dist/$base.zip" ]; then src="$REPO_ROOT/dist/$base.zip"
  elif [ -d "$REPO_ROOT/dist/Win64/Windows" ]; then src="$REPO_ROOT/dist/Win64/Windows"
  elif [ -d "$REPO_ROOT/dist/Mac/Mac" ]; then src="$REPO_ROOT/dist/Mac/Mac"; extra+=(--fix-permissions)
  fi
fi
if [ -z "$src" ] || [ ! -e "$src" ]; then fail "Nothing to upload. Run package + zip-check first, or pass a path."; exit 1; fi

if [ -z "$version" ]; then
  sha="$(git -C "$REPO_ROOT" rev-parse --short HEAD 2>/dev/null || true)"
  version="$(date +%Y%m%d-%H%M)${sha:+-$sha}"
fi

step "butler push $src $target:$channel --userversion $version ${extra[*]+${extra[*]}}"
butler push "$src" "$target:$channel" --userversion "$version" ${extra[@]+"${extra[@]}"}
step "Channel status for $target:$channel"
butler status "$target:$channel"
user="${target%%/*}"; game="${target#*/}"
echo
echo "Now open https://$user.itch.io/$game, download the build, extract it into a NEW folder and run it."
echo "Check the downloaded filename: the final submission must be TeamName_GameName.zip (docs/submission-checklist.md)."
