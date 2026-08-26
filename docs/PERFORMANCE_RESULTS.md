# Native performance results

Measured on the development machine on 2026-08-26 using the SDL Vulkan driver
(`radv`), a Release build, an offscreen SDL_GPU color target, 120 warmup frames,
and 10,000 measured frames per representative workload. Each frame is deliberately synchronized with
`SDL_WaitForGPUIdle`, so `gpu_complete_wait` is a conservative no-overlap
completion cost rather than an asynchronous submit claim.

| View / viewport | Rml update mean / p95 | Record mean / p95 | GPU completion mean / p95 | Complete CPU frame mean / p95 |
|---|---:|---:|---:|---:|
| Play lobby, 1280×720 | 0.0025 / 0.0031 ms | 0.0915 / 0.1106 ms | 0.1006 / 0.1220 ms | 0.2055 / 0.2328 ms |
| Session rules with native controls, 1280×720 | 0.0033 / 0.0036 ms | 0.1092 / 0.1123 ms | 0.0983 / 0.1159 ms | 0.2215 / 0.2433 ms |
| Controls/bindings, 1280×720 | 0.0036 / 0.0038 ms | 0.1222 / 0.1257 ms | 0.1034 / 0.1228 ms | 0.2404 / 0.2611 ms |
| Controls/input tuning, 1280×720 | 0.0022 / 0.0024 ms | 0.0923 / 0.0951 ms | 0.0963 / 0.1127 ms | 0.2016 / 0.2220 ms |
| 20-mod catalog, 1280×720 | 0.0042 / 0.0044 ms | 0.1644 / 0.1684 ms | 0.1132 / 0.1300 ms | 0.2937 / 0.3125 ms |
| 20-mod catalog, 1920×1080 | 0.0045 / 0.0047 ms | 0.1830 / 0.1899 ms | 0.1557 / 0.1756 ms | 0.3559 / 0.3819 ms |

For the populated catalog at 1280×720:

- cold document, layout, and twenty-image route ready: 35–36 ms in the final
  runs (the unoptimized source images took roughly 153 ms);
- already-resident hide mean / p95: 0.0090 / 0.0074 ms;
- already-resident show mean / p95: 0.0099 / 0.0100 ms;
- document close: 0.013–0.014 ms;
- whole-process RSS: about 89,736–89,940 KiB;
- RSS increase across document and populated route load: about 8,292–8,404 KiB;
- current unstripped statically linked experiment executable: 8.1 MiB.

The RSS delta is not “the object size.” It includes document allocations,
decoded fonts and artwork, renderer geometry/staging allocations, allocator
slack, and memory committed by the graphics stack during the phase. The full
RSS also includes SDL, RmlUi, ImGui, the Vulkan driver, shared mappings, and the
executable. Object-level retained bytes still require a counting allocator or
heap profiler.

## Interpretation

The realistic catalog remains far inside the provisional sub-1-ms update and
sub-3-ms render budgets. Its conservative synchronized complete-frame mean is
0.294 ms at 720p (about 3,400 theoretical uncapped frames/s) and 0.356 ms at
1080p (about 2,800/s). A 144 Hz frame is 6.94 ms, so the 1080p catalog p95 uses
about 5.5% of that frame. Its largest observed complete-frame outlier across
10,000 samples was 2.904 ms, still just inside the 3 ms UI budget. A resident
shell can be shown or hidden far inside one 144 Hz frame.
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
