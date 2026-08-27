# Gubsy Dear ImGui trial

The shared Gubsy shell implemented as a full-screen Dear ImGui application on
SDL3 + SDL_GPU. Docking is not used; this is an embedded game menu, not a debug
tool window.

```sh
./scripts/run.sh
./scripts/run.sh --resolution 1920x1080
./scripts/run.sh --screen mods
./scripts/run.sh --benchmark 2000
```

Dear ImGui's keyboard and gamepad navigation are enabled. Real combo boxes,
sliders, checkboxes, tabs, child scrolling regions, and local state are used.
The fixed game viewport never scrolls as a whole; only intentionally dense list
and detail panes scroll. Catalog rows and detail views use the reference mod
artwork, while navigation icons are drawn as resolution-independent geometry.
