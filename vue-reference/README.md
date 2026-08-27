# Gubsy UI Design Demo

A complete browser-local interaction mock for the proposed reusable Gubsy game
shell. It reimagines the current Splonks menu workflows with stable, visible
navigation rather than copying the existing folder-like layout.

## Run

```bash
./scripts/run.sh
```

Open the URL printed by Vite (normally <http://localhost:5173>). The script
installs dependencies the first time. To test the production bundle:

```bash
npm run build
npm run preview
```

## Controls

- Arrow keys: move through the geometric focus graph.
- Enter: activate the focused control.
- Escape or Backspace: cancel/return to Play.
- Q / E: previous/next local subsection, standing in for LB/RB.
- Pointer input works normally and switches the visible input-ownership mode.

The `Demo state` selector exposes populated, empty, loading, and recoverable
error states. All edits persist to `localStorage`. Settings → Display includes a
button to restore the original fake dataset.

## Covered workflows

- A Valve-style persistent Play lobby combining continue/new-game activity,
  solo/local/friends/public access, automatic/direct/relay/dedicated/Steam host
  choice, roster, invites, game settings, session mods, joining, and launch.
- A game-provided Splonks Play implementation rather than a universal lobby:
  new expeditions choose authored quests and inspect their stage route, continued
  expeditions choose a recorded checkpoint, and Arena exposes a different
  session payload and twelve match rules. Gubsy owns the surrounding shell and
  reusable editors, while each game owns the activity schema and composition.
- Local players, profiles (create/select/rename/reset/delete), device assignment,
  roster readiness and refresh.
- Display, audio, accessibility, and gameplay settings with all basic widget
  types and a final-resolution preview.
- Binding profiles (create/select/rename/reset/delete), action filtering,
  multiple bindings, add/replace/remove capture flow, and per-device tuning.
- Offline lobby, session settings, local/direct/room hosting, address/code join,
  server browser, connecting/waiting/host states, and full-room handling.
- Profile identity/history separated from game-provided progression and nested
  automatic checkpoints. The Play picker stays inside the lobby while the
  Progress destination handles supported management operations.
- Device-level installed-mod/version management and a searchable 20-entry
  illustrated catalog. Required and optional dependencies are visible, required
  packages install recursively, and dependent removal requires confirmation.
- A full Play-lobby mod workspace where catalog content can be installed and
  activated in one action. Progression records exact package/version manifests
  and can restore its set without overwriting that record.
- The same mod workspace is reachable from the running-session pause menu. Mods
  declare hot-safe, next-stage, or new-session lifecycle behavior; unsafe changes
  can be queued, applied while preserving namespaced state, or removed with an
  explicit state/entity cleanup.
- Typed settings contributed by active mods appear in the game-provided session
  editor with visible source attribution. The 1280×720/tablet breakpoint keeps
  the list and detail inspector side-by-side with contained scrolling.
- Responsive full-game desktop (including 1920×1080), compact 1280×720,
  tablet, phone-landscape, and phone-portrait compositions. Phone layouts use
  safe-area-aware bottom navigation, 44px minimum visible controls, larger text,
  Setup/Party switching, and one-pane master/detail navigation for quests, rules,
  and session mods.

## Responsive test matrix

Layout breakpoints follow available CSS space rather than named hardware models.
The current acceptance samples are 1920×1080 and 1280×720 at DPR 1, 844×390 and
915×412 at DPR 3 in landscape, and 412×915 at DPR 3 in portrait. The page uses a
device-width viewport with `viewport-fit=cover`; DOM layout remains in CSS pixels
while the browser maps those pixels to the display density.

This is intentionally a UX/state prototype. It does not import Arbor or Gubsy;
its approved interaction and layout behavior will become the acceptance target
for the later native shell.

The mod catalog artwork is stored as four five-thumbnail sprite sheets generated
for this prototype with OpenAI's built-in image generation tool. It contains no
third-party game or mod artwork.
