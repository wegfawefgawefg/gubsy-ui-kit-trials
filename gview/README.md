# GView trial

This is the complete native Gubsy shell reference implemented with standalone
GLayout and GView over SDL3. It shares the content and behavior target used by
the Vue and RmlUi trials while exercising renderer-neutral layout, controls,
focus, scrolling, and paint commands.

```sh
./scripts/run.sh
./scripts/run.sh --screen 16
./scripts/run.sh --screen 17
./scripts/run.sh --editor
./scripts/run.sh --resolution 1920x1080
./scripts/run.sh --hidden --screen 9 --capture /tmp/gview.bmp
./scripts/run.sh --hidden --benchmark --scenario scroll --screen 16
```

Screen indices are:

| Index | State |
|---:|---|
| 0-3 | Play lobby, quest picker, expedition rules, session mods |
| 4-6 | Local players, profiles, devices |
| 7-10 | Display, audio, accessibility, gameplay settings |
| 11-13 | Bindings, control devices, input tuning |
| 14 | Progress, campaigns, checkpoints, recorded package set |
| 15-16 | Installed mods and catalog |
| 17 | Non-menu inventory over a native custom world surface |

Every overflow region is an internal GView scroll area. The shell has no outer
document scroll. Widths below 1000 or heights below 600 select the compact
composition used for tablet, landscape phone, and portrait evidence.

Mouse controls use normal hit testing. Keyboard and gamepads produce semantic
navigation actions; controllers are opened on startup and hot-plugged. Focus
groups provide generated local geometric movement, group-level exits, exact
remembered-member re-entry, explicit exceptional edges, dropdown capture/cancel,
and Back-to-owner behavior instead of moving a synthetic pointer or persisting
fake-data item IDs.

F3 toggles the live authoring suite. Test mode sends input to the UI; Edit mode
pauses it and makes the polished native canvas directly selectable. The canvas
supports clean, layout, focus, and combined overlays; center/edge/corner drag,
grid and sibling snapping, nudge, multi-select, reparent, copy/cut/paste,
duplicate/delete, undo/redo, save, and reload. Focus links are staged on the
canvas and require explicit Apply or Cancel. Separate focused ImGui windows
select state/provider data, edit properties/groups/themes, inspect timings, and
run the 36-preset display simulator with independent logical/physical size,
device pixel ratio, UI scale, safe area, fit mode, sampling, zoom, and pan.
Authoring uses the same View and S-expression representation as runtime.

Benchmark scenarios are `stable`, `value`, `layout`, and `scroll`. The first
120 frames are discarded. Output separates runtime update, SDL recording,
complete CPU frame, compile/activation, cache builds, and GView-owned memory.

The window is created hidden as an SDL utility, positioned on the leftmost
display, and only then shown. Hidden capture and benchmark modes never take
pointer or keyboard focus.
