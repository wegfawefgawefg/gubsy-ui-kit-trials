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

- Main shell, continue/new game, local co-op, online entry, quit confirmation.
- Local players, profiles (create/select/rename/reset/delete), device assignment,
  roster readiness and refresh.
- Display, audio, accessibility, and gameplay settings with all basic widget
  types and a final-resolution preview.
- Binding profiles (create/select/rename/reset/delete), action filtering,
  multiple bindings, add/replace/remove capture flow, and per-device tuning.
- Offline lobby, session settings, local/direct/room hosting, address/code join,
  server browser, connecting/waiting/host states, and full-room handling.
- Save catalog, new/load/rename/copy/delete, incompatible data, and confirmations.
- Installed mod list, compatibility, enable/disable, refresh, and browse hook.
- Responsive desktop, compact desktop/4:3, and narrow layouts.

This is intentionally a UX/state prototype. It does not import Arbor or Gubsy;
its approved interaction and layout behavior will become the acceptance target
for the later native shell.
