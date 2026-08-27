# Performance and memory plan

## Budgets under evaluation

- Normal UI update/layout: comfortably below 1 ms on the target development
  machine.
- UI render recording/submission contribution: below 3 ms, leaving meaningful
  room in a 16.67 ms game frame.
- Show or hide a previously prepared complex menu within one 144 Hz frame
  (6.94 ms), with 60 Hz (16.67 ms) as the minimum game-loop requirement.
- No recurring allocations or geometry rebuilds for an unchanged static screen.

The protocol remains the source of truth; measured results from the completed
native route matrix are recorded in [PERFORMANCE_RESULTS.md](PERFORMANCE_RESULTS.md).

## What memory number means

`VmRSS` is resident memory attributed to the whole process. It includes the
executable and shared mappings, SDL, the graphics driver, GPU staging/mapped
memory, RmlUi, ImGui, assets, allocator slack, and the document objects. It is
not "the binary" and it is not the size of one UI object.

Measure these deltas separately:

1. process start before SDL/GPU;
2. SDL window and GPU device initialized;
3. RmlUi initialized with an empty context;
4. shell and fonts loaded;
5. each representative screen loaded but hidden;
6. screen shown and warmed;
7. screen destroyed and allocator settled.

For object-level consumption, add a counting allocator around RmlUi allocations
or use heap profiling. RSS alone cannot answer that question.

## Timing protocol

Use Release builds and report p50, p95, p99, and maximum over at least 10,000
warm frames. Keep these spans distinct:

- event dispatch;
- fake/Gubsy state mutation;
- RmlUi `Update()` (binding, style, layout);
- render command/geometry recording;
- ImGui instrumentation overhead;
- CPU submit/wait;
- GPU duration when timestamp support is available;
- complete CPU frame.

Run once with normal vsync/presentation for player-visible pacing and once with
presentation uncapped for CPU throughput. Do not mistake a blocking present for
UI render cost.

## Workloads

- Empty shell baseline.
- Stable Play lobby.
- Session rules with all rows and mod-contributed controls.
- Controls with the full action/binding set and a live device monitor.
- Installed mod detail with dependency/dependent graphs.
- Twenty-entry catalog with thumbnails and internal scrolling.
- A stress catalog/inventory with hundreds and then thousands of rows to expose
  the cost model and determine whether list virtualization is necessary.
- Modal open/close, subsection swap, viewport reflow, DPI change, text editing,
  slider drag, rapid controller traversal, and repeated show/hide.

Measure cold document parsing separately from warm show/hide. The desired
one-frame menu transition can be achieved through prepared hidden documents or
caching only if their retained memory is also acceptable.

## Further instrumentation for a Gubsy integration

- Persistent ring-buffer telemetry for an in-game overlay. The CLI benchmark
  already accumulates samples without formatting each frame.
- RmlUi render-interface counters for draw commands, vertices, indices, texture
  switches, clips, compiled geometry, and bytes uploaded.
- Allocation counts/bytes by phase and live retained bytes per document.
- Optional Tracy zones for event, update, layout-triggering mutations, render,
  upload, and submit.
- CSV/JSON output containing build commit, screen/state, resolution, DPI,
  renderer/driver, vsync mode, samples, and memory deltas.

The benchmark overlay itself must be removable so its overhead can be measured
and excluded.
