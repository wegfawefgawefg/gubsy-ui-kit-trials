# Gubsy Nuklear trial

The shared Gubsy shell implemented with Nuklear's single-header immediate-mode
API and its small SDL/OpenGL reference backend. This pass evaluates widget and
layout ergonomics, content density, controls, binary/RSS cost, and frame cost.

```sh
./scripts/run.sh
./scripts/run.sh --resolution 1920x1080
./scripts/run.sh --screen mods
./scripts/run.sh --benchmark 2000
```

Nuklear is renderer neutral. A production choice would receive an SDL3/Gubsy
renderer adapter after this UI-level trial, rather than during it.
