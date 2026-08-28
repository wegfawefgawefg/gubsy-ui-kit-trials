#!/usr/bin/env bash
set -euo pipefail

trial_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cmake -S "$trial_dir" -B "$trial_dir/build" -G Ninja
cmake --build "$trial_dir/build" -j
exec "$trial_dir/build/gubsy-gview-trial" --width 1280 --height 720 "$@"
