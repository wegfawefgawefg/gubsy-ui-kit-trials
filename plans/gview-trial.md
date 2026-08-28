# GView trial contract

## Milestone status

The first complete trial is implemented. All 18 states above render with real
controls and assets; the hidden controller self-test traverses every route and
checks group memory, explicit dropdown commit/cancel, a real slider, text input,
and the inventory action scope. Fresh 720p and 1080p visual sweeps, compact
landscape/portrait captures, strict builds, and benchmark scenarios have been
inspected. Measured results and the updated recommendation live in
`evaluation-notes/GVIEW_RESULTS.md` and `CURRENT_COMPARISON.md`.

This milestone is the reference implementation for the libraries, not the end
of UI design evaluation. A diverse game-UI suite and final Splonks migration
remain deliberate later work after user review.

The authoring/presentation recovery pass is implemented and awaiting hands-on
user acceptance. It adds native-canvas editing, compiled asset presentation
recipes, corrected density-aware text, safe group focus editing, all 36 legacy
display presets with logical/physical output separation, and a real
Gubsy-runtime host service. The durable implementation and acceptance record is
`../../gview/docs/AUTHORING_PRESENTATION_PLAN.md`.

## Purpose

Add a complete GView implementation to the shared native UI comparison. The
Vue mock and RmlUi trial are the primary behavior/content references.

The GView version is not allowed to shrink the interface to accommodate missing
runtime capabilities. Missing reusable capabilities are implemented in GLayout
or GView and documented before the trial is considered complete.

## Required views

- Play, including continue/new expedition differences.
- Quest/checkpoint selector.
- Expedition settings, including game and mod-contributed rules.
- Full session-mod management and browse/install flow.
- Players: local players, profiles, and input-device assignment.
- Settings: display, audio, accessibility, and gameplay.
- Controls: bindings, devices, manual/listen input, and input tuning.
- Progress: profiles, campaigns/checkpoints, metadata, and package sets.
- Mods: installed management, catalog, compatibility, dependencies, images.
- A non-menu game UI proof using a HUD/inventory/custom surface.

## Interaction quality

- Real buttons, selects, toggles, sliders, inputs, scrolling, and overlays.
- Mouse interaction and full controller reachability.
- Local focus relationships, scopes, memory, modal containment, and back policy.
- Dynamic lists/grids generate relationships without per-item hardcoding.
- No dead controls, click-to-cycle fake selects, focus traps, or accidental
  cross-screen movement.
- State mutations retain relevant scroll and focus position.

## Visual quality

- Production layouts at 1280x720 and 1920x1080.
- Correct DPI scaling, text metrics, baselines, and centering.
- Images/assets and custom-surface content are present.
- No accidental outer scrolling, overlaps, clipped values, or scrollbar gutters
  covering content.
- Compact evidence is included where it exercises a distinct layout variant.

The trial may use a neutral theme. Exact colors are less important than layout,
content, responsiveness, assets, interaction feedback, and legibility.

Regression coverage includes the former bottom-clipped text, overlapping
slider parts, unclear select/destination affordances, unpadded transparent
popups, full-screen `Player added` toast, and weak panel hierarchy. These were
fixed in shared metrics, compound-widget, portal, overlay, and theme models
rather than with per-screen offsets.

## Reusable-boundary rule

- Geometry generally useful to HUD/settings/inventory belongs to GLayout.
- Presentation/interaction generally useful across games belongs to GView.
- Reference-game meaning remains in the trial.
- Unusual rendering uses a generic custom-surface API.

Do not add Splonks-specific library widgets to achieve parity.

## Authoring evidence

The running trial must expose the optional authoring suite:

- View, nested state, and fake scenario selection.
- Internal/window resolution, aspect, DPI, form factor, and safe-area controls.
- Live hierarchy and layout overlays.
- Create, edit, reparent, undo/redo, save, and reload.
- Directed focus-graph inspection/editing and immediate controller testing.
- Performance/allocation inspection.

This evidence must operate over the actual native rendering. The author can
freeze normal input, directly drag/resize/reparent real nodes, toggle clean,
layout, navigation, and combined overlays, and safely author group-level focus
entry/exit and remembered-member policy. Detached ImGui diagrams alone do not
satisfy this requirement.

The standalone trial remains the complete visual and clean performance path.
`gubsy::ui::ViewRuntime` supplies the second path inside a real `GubsyRuntime`
using normal events, mapped input, devices, typed model behavior, asset domains,
and display services. The engine-lifecycle smoke validates the host boundary
without copying the reference shell into another executable.

## Benchmarks

Use the shared benchmark harness and record:

- 720p and 1080p Play and dense Mods screens.
- Stable, value-mutating, layout-mutating, and scrolling scenarios.
- Parse/compile and activation time.
- Layout, compose, text, renderer, and full-host timings.
- Whole-process RSS and library-owned memory.
- Binary size and allocations.

The target is dense 1080p below 3 ms with comfortable 144 Hz headroom and
normal update/layout below 1 ms. Results must remain comparable to RmlUi,
Arbor, Dear ImGui, raygui, and Nuklear.

## Code quality

Follow the Adventures-with-Chickens/Gubsy policy:

- Roughly 300-500 lines maximum per source file.
- Cohesive domains and mostly flat organization.
- Direct, debuggable C++.
- Terse what-is comments above paragraph blocks.
- No giant host, screen, or catch-all utility files.

## Completion

Completion requires visual inspection of every main and nested view, automated
behavior/graph checks, clean builds, benchmark evidence, documentation,
committed/pushed repositories, and a running floating trial for user review.
