#!/usr/bin/env bash
# Shared helpers for tools/*.sh. Source it:  . "$(dirname "$0")/common.sh"
# Provides: TOOLS_DIR, REPO_ROOT, jam_load_env, jam_setting, jam_zip_base, human_size, file_size, step, warn, fail
TOOLS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$TOOLS_DIR/.." && pwd)"

jam_load_env() {
  # Load KEY=VALUE lines from tools/jam.env into the environment. Existing environment variables win.
  local file="$TOOLS_DIR/jam.env" line key value
  [ -f "$file" ] || return 0
  while IFS= read -r line || [ -n "$line" ]; do
    case "$line" in ''|\#*) continue ;; esac
    case "$line" in *=*) ;; *) continue ;; esac
    key="$(printf '%s' "${line%%=*}" | tr -d '[:space:]')"
    value="${line#*=}"
    value="${value#"${value%%[![:space:]]*}"}"
    value="${value%"${value##*[![:space:]]}"}"
    value="${value%\"}"; value="${value#\"}"; value="${value%\'}"; value="${value#\'}"
    [ -n "$key" ] || continue
    if [ -z "${!key:-}" ] && [ -n "$value" ]; then export "$key=$value"; fi
  done < "$file"
}

jam_setting() { # jam_setting NAME [DEFAULT]
  local v="${!1:-}"
  printf '%s' "${v:-${2:-}}"
}

jam_zip_base() {
  local team game
  team="$(jam_setting JAM_TEAM_NAME | tr -cd 'A-Za-z0-9')"
  game="$(jam_setting JAM_GAME_NAME | tr -cd 'A-Za-z0-9')"
  if [ -z "$team" ] || [ -z "$game" ]; then
    echo "Set JAM_TEAM_NAME and JAM_GAME_NAME in tools/jam.env (only letters and digits survive)." >&2
    return 1
  fi
  printf '%s_%s' "$team" "$game"
}

human_size() { # bytes -> human readable (binary units)
  awk -v b="$1" 'BEGIN { split("B KiB MiB GiB", u, " "); i = 1; while (b >= 1024 && i < 4) { b /= 1024; i++ } printf (i == 1 ? "%d %s" : "%.1f %s"), b, u[i] }'
}

file_size() { stat -f%z "$1" 2>/dev/null || stat -c%s "$1"; }
step() { printf '\033[36m==> %s\033[0m\n' "$*"; }
warn() { printf '\033[33mWARNING: %s\033[0m\n' "$*" >&2; }
fail() { printf '\033[31mFAIL: %s\033[0m\n' "$*" >&2; }
