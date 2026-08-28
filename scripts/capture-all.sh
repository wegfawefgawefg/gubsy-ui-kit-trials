#!/usr/bin/env bash
set -euo pipefail

workspace_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
output_dir=${1:-"$workspace_dir/evaluation-notes/captures"}
screens=(play players settings controls progress mods)
viewports=(1280x720 1920x1080 915x412 412x915)

for trial in dear-imgui raygui nuklear gview; do
  cmake -S "$workspace_dir/$trial" -B "$workspace_dir/$trial/build" -G Ninja -DCMAKE_BUILD_TYPE=Release
  cmake --build "$workspace_dir/$trial/build"
done
cmake -S "$workspace_dir/arbor" -B "$workspace_dir/arbor/build" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "$workspace_dir/arbor/build"

for viewport in "${viewports[@]}"; do
  mkdir -p "$output_dir/$viewport"
  index=0
  for screen in "${screens[@]}"; do
    "$workspace_dir/arbor/scripts/capture.sh" \
      "$output_dir/$viewport/arbor-$screen.png" "$screen" "$viewport"
    "$workspace_dir/dear-imgui/build/gubsy-dear-imgui-trial" \
      --resolution "$viewport" --screen "$screen" \
      --capture "$output_dir/$viewport/dear-imgui-$screen.bmp"
    "$workspace_dir/raygui/build/gubsy-raygui-trial" \
      --resolution "$viewport" --screen "$screen" \
      --capture "$output_dir/$viewport/raygui-$screen.png"
    "$workspace_dir/nuklear/build/gubsy-nuklear-trial" \
      --resolution "$viewport" --screen "$screen" \
      --capture "$output_dir/$viewport/nuklear-$screen.bmp"
    gview_screens=(0 4 7 11 14 16)
    width=${viewport%x*}
    height=${viewport#*x}
    "$workspace_dir/gview/build/gubsy-gview-trial" --hidden \
      --width "$width" --height "$height" --screen "${gview_screens[$index]}" \
      --capture "$output_dir/$viewport/gview-$screen.bmp"
    index=$((index + 1))
  done
done

printf 'Captured %d backend/route/viewport frames in %s\n' "$((5 * 6 * 4))" "$output_dir"
