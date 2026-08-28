#!/usr/bin/env bash
set -euo pipefail

workspace_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
output=${1:-"$workspace_dir/evaluation-notes/benchmark-raw.txt"}
: > "$output"

for viewport in 1280x720 1920x1080; do
  printf 'viewport=%s\n' "$viewport" | tee -a "$output"
  "$workspace_dir/rmlui/build/gubsy-rmlui-demo" --benchmark 1000 \
    --resolution "$viewport" --screen 0 | tee -a "$output"
  width=${viewport%x*}
  height=${viewport#*x}
  for scenario in stable value layout scroll; do
    screen=0
    if [[ "$scenario" == scroll ]]; then screen=16; fi
    "$workspace_dir/gview/build/gubsy-gview-trial" --benchmark --hidden \
      --width "$width" --height "$height" --screen "$screen" \
      --scenario "$scenario" | tee -a "$output"
  done
  "$workspace_dir/arbor/build/gubsy-arbor-trial" --benchmark 1000 \
    --resolution "$viewport" --screen play | tee -a "$output"
  "$workspace_dir/dear-imgui/build/gubsy-dear-imgui-trial" --benchmark 1000 \
    --resolution "$viewport" --screen play | tee -a "$output"
  "$workspace_dir/raygui/build/gubsy-raygui-trial" --benchmark 1000 \
    --resolution "$viewport" --screen play | tee -a "$output"
  "$workspace_dir/nuklear/build/gubsy-nuklear-trial" --benchmark 1000 \
    --resolution "$viewport" --screen play | tee -a "$output"
done

for trial in rmlui arbor dear-imgui raygui nuklear gview; do
  binary=$(find "$workspace_dir/$trial/build" -maxdepth 1 -type f -perm -111 -name 'gubsy-*' -print -quit)
  if [[ -n "$binary" ]]; then
    printf 'binary=%s bytes=%s\n' "$trial" "$(stat -c %s "$binary")" | tee -a "$output"
  fi
done
