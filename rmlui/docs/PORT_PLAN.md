# Port plan

## Purpose

Recreate the current Vue Gubsy shell as a native RmlUi/SDL3 application closely
enough that the browser mock remains useful as the design tool and acceptance
target. The experiment should answer these questions before any Arbor rewrite or
Gubsy-wide integration is attempted:

1. Can RmlUi express the approved desktop, compact 1280×720, landscape-phone,
   and portrait-phone compositions without fragile layout hacks?
2. Can it preserve the important stateful workflows, focus behavior, controller
   navigation, and manual input-binding tooling?
3. Can a realistic heavy screen stay within the CPU, GPU, memory, and one-frame
   show/hide budgets?
4. Is the resulting native authoring burden acceptable compared with a later
immediate-mode implementation of the same target?

## Implementation status

The first complete parity pass is implemented. All seventeen committed states
render at all four viewport families; the shell, route state, focus graph,
keyboard/controller handling, provider states, modal/toast layers, internal
scrolling, real catalog artwork, and SDL_GPU/ImGui composition are native.
`scripts/validate.sh` exercises the route/input state machine and all 68
route/viewport combinations without displaying a window. Remaining work is
evaluation and integration design, not filling placeholder pages.

## Deliberate architecture

```text
SDL events ──┬── ImGui experiment panel
             ├── RmlUi DOM/input
             └── later: Gubsy action + raw-device adapters

game state / fake providers ── data models ── RmlUi documents

SDL_GPU command buffer ── game/background ── RmlUi ── ImGui telemetry
```

The current host owns SDL directly and instantiates RmlUi's platform and
SDL_GPU render interfaces rather than using its standalone sample-backend
singleton. That is required for native compositing and lets both UI libraries
use one device and one submission.

No generic renderer abstraction, Arbor compatibility layer, AXL translator, or
Gubsy package API belongs in this experiment yet. Those would hide the answer to
the narrower feasibility question.

## Port phases

### 0. Reproducible native baseline

- Build SDL_GPU + RmlUi + ImGui in one executable.
- Provide viewport/DPI/screen controls and initial frame/RSS telemetry.
- Capture the Vue reference matrix.
- Establish a stable fake-state model and screen registry.

### 1. Shell and responsive composition

- Port top status bar, stable desktop rail, content region, prompt bar, modal
  layer, and notifications.
- Port bottom navigation and safe-area behavior for phone layouts.
- Implement contained scrolling: the outer game surface must never scroll.
- Treat 1920×1080 and 1280×720 as first-class desktop/game layouts, not scaled
  copies. Treat 915×412 and 412×915 as distinct compositions.

RmlUi supports flex layout and media queries but not browser CSS Grid. Existing
grid compositions must become explicit flex rows/columns or small native layout
components. Do not emulate grid with hard-coded absolute coordinates.

### 2. Play provider and nested workflows

- Lobby setup/party composition.
- Continue checkpoint and new-quest route browsers.
- Full game-provided expedition/match rules editor, including mod-contributed
  settings and source attribution.
- Session mod current-set and browse-and-add workflows.
- Pause/in-game entry points and runtime mod policy prompts.

This phase proves that the hardest reactive nested menus work before spending
time on simpler settings pages.

### 3. Players, identity, and progression

- Local roster, profiles/history, and device assignment.
- Multiple input devices owned by one local player.
- Profile data distinct from game-provided progression/checkpoints.
- Checkpoint history and exact recorded mod manifests.

### 4. Controls

- Binding profile lifecycle, action filtering, multiple bindings, and conflicts.
- Both capture styles: "perform an input" and explicit manual device/control
  selection. Neither replaces the other.
- Digital, 1D axis, 2D axis, trigger-to-button, button-to-axis, chords, and
  transform/qualifier editing as exposed by Gubsy.
- Device browser with live raw-input exploration so unknown wheels, pedals,
  sticks, and macro pads can be identified.
- Geometric focus graph and Gubsy action integration, kept separate from raw
  binding capture.

### 5. Settings and mod library

- Display/audio/accessibility/gameplay settings and all widget types.
- Installed package/version/dependency management.
- Twenty-entry illustrated catalog, compatibility filtering, dependency plans,
  uninstall dependent handling, and install-and-activate flows.

### 6. Native acceptance and benchmark

- Complete pointer, keyboard, controller, text entry, scroll, modal, and focus
  traversal passes.
- Compare every native state against its matching committed Vue reference.
- Run the protocol in `PERFORMANCE_PLAN.md` on light and worst-case states.
- Record authoring pain, layout workarounds, missing primitives, and renderer
  limitations honestly.

Only after this phase should we choose among: adopt RmlUi, fork/extend it, merge
ideas into Arbor, or build the same approved shell with an immediate-mode UI.

## Definition of complete

- Every row in `PARITY_MATRIX.md` is implemented or has an explicit documented
  native deviation accepted for a concrete reason.
- The four viewport families are usable with no outer scrolling, clipped text,
  inaccessible controls, or pointer-only path.
- Complex lists have bounded internal scrolling and stable selection/detail
  behavior.
- Show/hide, update, render, allocations, and memory are measured on realistic
  populated screens rather than toy documents.
- The repo can be built and run from a fresh checkout using its documented
  commands.
