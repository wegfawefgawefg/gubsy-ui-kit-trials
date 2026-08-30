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
fake-data item IDs. Focus changes reveal their item in every owning scroll area,
and stable-ID runtime reconciliation preserves that scroll when an on-focus
selection rebuilds adjacent host content.

F1 is the master show/hide for the complete GView authoring layer. Its small
launcher only selects focused tool windows; native-canvas modes live in the
Canvas Editor window instead of crowding the launcher. F2 switches Test/Edit
mode, F3 toggles all layout boxes, F4 toggles the grid, and F5 toggles focus
relationships. Test mode sends input to the UI; Edit mode pauses it. The canvas supports
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
The simulator writes `authoring/display-simulator.sexp` whenever its logical or
output size, density, UI scale, form factor, safe area, presentation, sampling,
zoom, or pan changes. Opening the authoring tools in a later process restores
that profile; it does not resize the desktop host unless host-follow is
explicitly enabled.
Authoring uses the same View and S-expression representation as runtime. Each
screen keeps an independent in-memory layout/interaction document and history.
The project theme is a separate shared document: global, control-kind, and
style-class recipes immediately apply across every page, while exact-node
overrides remain local to the current page. The editor labels that storage
scope beside the selected recipe. Save writes both the current screen and
`authoring/shared-theme.sexp`; Reload intentionally discards both working
domains and reads them from disk. Before the shared document exists, the trial
migrates the newest legacy page theme once so previous tuning is not discarded.
Host model refreshes continue to update semantic content without replacing
unsaved authoring work. Flow-child dragging reorders siblings, edge/corner
dragging edits only the affected size axes, and absolute or anchored children
retain free positional editing.

Widget themes support natural, stretch, contain, cover, tile, and nine-slice
image modes. Nine-slice keeps authored corners intact while stretching the
edges and center, so the same asset can skin panels, buttons, slider tracks,
and slider fills at unrelated sizes. C++ recipes set `ImageMode::NineSlice`
and `PartPresentation::slice`; S-expression themes use `(image_mode
nine_slice)` and `(slice 16)`. The Theme & Assets window additionally edits
asymmetric source margins, rendered border scale, tint, and opacity live, and
can overlay slice guides on the native canvas. A native-canvas pick follows the
exact part and visual state that was painted, including a second pick on the
same node. By default, cuts, rendered scale, and tile modes propagate to the
other states of that part that use the same asset; green action and light row
assets therefore remain independently tunable. The trial sliders use tintable
parchment strips and knobs from the supplied control set. The same set skins
toggle on/off plates and passive scroll tracks/thumbs. Asymmetric nine-slice
margins give horizontal and vertical strips three-slice behavior without a
second rendering primitive; fixed-shape knobs and toggles use Contain.

The Theme & Assets editor targets recipes at any control, a control kind, a
style class, or one exact selected node. Class and node recipes can skin normal
layout containers, so section backgrounds support the same natural, stretch,
cover, tile, and nine-slice modes as widgets. Exact-node recipes override class
recipes, which override control-wide recipes. `Draw box underneath` only affects
nodes matched by that recipe; unmatched nodes retain their semantic fallback
box. Canvas selection now follows the effective shared recipe automatically,
including recipes inherited from `gubsy-default`. The editor shows recipes
applying to the selection, the complete inherited recipe library, source layer,
selector scope, parts, and match count. Clicking a compound widget follows the
specific frame, track, fill, thumb, or indicator under the pointer. Reusable
preset/classes can be assigned or created in place; exact-node overrides remain
under an advanced section.

Presented regions can independently layer Shadow, Background, and Frame parts,
so a backplate can sit behind a frame without also drawing the fallback
rectangle. Structural rows, workspaces, and spacers are layout-only nodes and
paint nothing. The current trial maps the project owner's parchment nine-slices
to semantic roles: dark chrome, ornate outer regions, inner groups, light rows,
and green action states. The square 256 px frames share a 23 px source inset
and render that border at 1.0 scale.

Each nine-slice edge and its center independently support Stretch, Repeat,
Mirror, Blank Repeat, and Hide modes. The live editor also controls whether the
normal semantic box is drawn underneath the asset. Navigation randomly
alternates the first two Wood Block sounds from Nathan Gibson's Universal UI
Soundpack; activation, opening, closing, and toggling use the third. The
retained license and attribution are beside those audio assets.

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
