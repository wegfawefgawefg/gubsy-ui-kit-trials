#!/usr/bin/env bash
set -euo pipefail

repo_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
cmake -S "$repo_dir" -B "$repo_dir/build" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "$repo_dir/build"

"$repo_dir/build/gubsy-rmlui-demo" --self-test --resolution 1280x720

for viewport in 1920x1080 1280x720 412x915 915x412; do
  for screen in $(seq 0 16); do
    "$repo_dir/build/gubsy-rmlui-demo" \
      --hidden --no-tools --frames 3 \
      --resolution "$viewport" --screen "$screen"
  done
  for provider in 1 2 3; do
    "$repo_dir/build/gubsy-rmlui-demo" \
      --hidden --no-tools --frames 3 \
      --resolution "$viewport" --screen 0 --provider "$provider"
  done
done

printf 'Validated the interaction self-test, all 68 route/viewport combinations, and 12 additional provider-state compositions.\n'
