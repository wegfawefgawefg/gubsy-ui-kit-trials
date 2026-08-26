# Native performance results

Measured on the development machine on 2026-08-26 using the SDL Vulkan driver
(`radv`), a Release build, an offscreen SDL_GPU color target, 120 warmup frames,
and 1,080 measured frames. Each frame is deliberately synchronized with
`SDL_WaitForGPUIdle`, so `gpu_complete_wait` is a conservative no-overlap
completion cost rather than an asynchronous submit claim.

| View / viewport | Rml update mean / p95 | Record mean / p95 | GPU completion mean / p95 | Complete CPU frame mean / p95 |
|---|---:|---:|---:|---:|
| Play lobby, 1280×720 | 0.0025 / 0.0030 ms | 0.0944 / 0.1108 ms | 0.0987 / 0.1149 ms | 0.2069 / 0.2387 ms |
| Session rules, 1280×720 | 0.0025 / 0.0026 ms | 0.0916 / 0.0947 ms | 0.0945 / 0.1104 ms | 0.1993 / 0.2247 ms |
| Controls/bindings, 1280×720 | 0.0032 / 0.0034 ms | 0.1205 / 0.1244 ms | 0.1054 / 0.1281 ms | 0.2400 / 0.2823 ms |
| 20-mod catalog, 1280×720 | 0.0042 / 0.0043 ms | 0.1642 / 0.1689 ms | 0.1132 / 0.1290 ms | 0.2931 / 0.3121 ms |
| 20-mod catalog, 1920×1080 | 0.0046 / 0.0049 ms | 0.1842 / 0.1899 ms | 0.1604 / 0.1805 ms | 0.3621 / 0.3983 ms |
| 20-mod catalog, 412×915 | 0.0043 / 0.0047 ms | 0.1447 / 0.1483 ms | 0.0900 / 0.1053 ms | 0.2502 / 0.2671 ms |
| 20-mod catalog, 915×412 | 0.0042 / 0.0045 ms | 0.1339 / 0.1372 ms | 0.0854 / 0.1025 ms | 0.2346 / 0.2545 ms |

For the populated catalog at 1280×720:

- cold document, layout, and twenty-image route ready: 34–42 ms across the
  final runs (the unoptimized source images took roughly 153 ms);
- already-resident hide mean / p95: 0.0091 / 0.0081 ms;
- already-resident show mean / p95: 0.0102 / 0.0103 ms;
- document close: 0.010 ms;
- whole-process RSS: about 88,864 KiB;
- RSS increase across document and populated route load: about 7,136 KiB;
- current unstripped statically linked experiment executable: 8.1 MiB.

The RSS delta is not “the object size.” It includes document allocations,
decoded fonts and artwork, renderer geometry/staging allocations, allocator
slack, and memory committed by the graphics stack during the phase. The full
RSS also includes SDL, RmlUi, ImGui, the Vulkan driver, shared mappings, and the
executable. Object-level retained bytes still require a counting allocator or
heap profiler.

## Interpretation

The realistic catalog remains far inside the provisional sub-1-ms update and
sub-3-ms render budgets. A resident shell can be shown or hidden far inside one
144 Hz frame, and its next fully synchronized frame is still around 0.3 ms.
Cold construction does **not** fit one frame when it also decodes a populated
illustrated catalog; a production integration should create the shell and
preload/stream catalog thumbnails during a loading phase, then keep it resident.
This demonstrates feasibility, not final Gubsy integration cost: game
compositing, real provider mutation, larger textures, and hundreds or thousands
of rows require separate measurement. A stress/virtualization pass is still
appropriate before treating unbounded inventories or mod catalogs as solved.

Reproduce one workload with:

```bash
./scripts/run.sh --benchmark 1200 --resolution 1280x720 --screen 16
```
