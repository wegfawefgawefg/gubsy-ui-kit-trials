#!/usr/bin/env bash
set -euo pipefail

workspace_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
trial=${1:-}
if [[ -z "$trial" ]]; then
  printf 'usage: %s {rmlui|arbor|dear-imgui|raygui|nuklear|gview|vue-reference} [arguments...]\n' "$0" >&2
  exit 2
fi
shift

case "$trial" in
  rmlui|arbor|dear-imgui|raygui|nuklear|gview)
    exec "$workspace_dir/$trial/scripts/run.sh" "$@"
    ;;
  vue-reference)
    exec "$workspace_dir/vue-reference/scripts/run.sh" "$@"
    ;;
  *)
    printf 'unknown trial: %s\n' "$trial" >&2
    exit 2
    ;;
esac
