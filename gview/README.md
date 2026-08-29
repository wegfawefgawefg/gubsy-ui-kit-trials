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

Mouse controls use normal hit testing. Arrow keys and WASD produce the same
semantic navigation actions; gamepads are opened on startup and hot-plugged. Focus
groups provide generated local geometric movement, group-level exits, exact
remembered-member re-entry, explicit exceptional edges, dropdown capture/cancel,
and Back-to-owner behavior instead of moving a synthetic pointer or persisting
fake-data item IDs.

F1 toggles a small top-level tool launcher. F2 switches Test/Edit mode, F3
toggles all layout boxes, F4 toggles the grid, and F5 toggles focus relationships.
The launcher independently opens the hierarchy/properties, display simulator,
and focus inspector windows. Hiding every ImGui window does not leave Edit mode,
so the polished canvas remains directly manipulable without panels covering it.
Test mode sends input to the UI; Edit mode pauses it. The canvas supports
independent boxes, IDs, grid, and focus overlays; center/edge/corner drag,
grid and sibling snapping, nudge, multi-select, reparent, copy/cut/paste,
duplicate/delete, undo/redo, save, and reload. Focus links are staged on the
canvas and require explicit Apply or Cancel. Separate focused ImGui windows
select state/provider data, edit properties/groups/themes, inspect timings, and
run the 36-preset display simulator with independent logical/physical size,
device pixel ratio, UI scale, safe area, fit mode, sampling, zoom, and pan.
Device presets remain fitted inside the current desktop window; simulated
physical output never silently resizes the host window. The host can explicitly
follow a fraction of logical size or match only its aspect ratio. The simulated
canvas and ImGui tools use separate presentation layers, so tiny retro and large
high-density presets leave tool size and mouse coordinates unchanged. For a
direct compositor check, pass `--logical-resolution 160x144` while keeping the
normal `--width 1280 --height 720` host.
Authoring uses the same View and S-expression representation as runtime.

Widget themes support natural, stretch, contain, cover, tile, and nine-slice
image modes. Nine-slice keeps authored corners intact while stretching the
edges and center, so the same asset can skin panels, buttons, slider tracks,
and slider fills at unrelated sizes. C++ recipes set `ImageMode::NineSlice`
and `PartPresentation::slice`; S-expression themes use `(image_mode
nine_slice)` and `(slice 16)`. The trial sliders use tintable CC0 stepped-panel
assets derived from Kenney Fantasy UI Borders; their license is retained beside
the assets.

Navigation is authored between semantic scopes rather than repeated item
pairs. Tabs, toolbars, primary lists, detail panes, and action rows remember
their own last member. Selecting a layout container in the focus inspector can
assign its complete focusable subtree as one scope. F5 labels pink scope exits
and cyan item exceptions directly over the rendered controls; unlabeled local
movement is generated spatially inside the outlined scope.

Benchmark scenarios are `stable`, `value`, `layout`, and `scroll`. The first
120 frames are discarded. Output separates runtime update, SDL recording,
complete CPU frame, compile/activation, cache builds, and GView-owned memory.

The window is created hidden as an SDL utility, positioned on the leftmost
display, and only then shown. Hidden capture and benchmark modes never take
pointer or keyboard focus.
