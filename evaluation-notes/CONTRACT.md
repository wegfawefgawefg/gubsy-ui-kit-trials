# Shared evaluation contract

Each trial must demonstrate the same useful shell rather than a toolkit's toy
widgets page.

## Required destinations

1. Play: continue/new activity choice, checkpoint/quest selection, host mode,
   party, session rules, and session mods.
2. Players: local-player assignment, persistent profiles, and input devices.
3. Settings: display, audio, accessibility, and gameplay categories with real
   toggles, dropdowns, and sliders.
4. Controls: bindings, manual/listen binding workflows, device assignment, and
   input tuning.
5. Progress: campaigns/checkpoints, profile ownership, recorded mod set, and
   resume/export actions.
6. Mods: installed management plus a populated catalog with compatibility and
   dependency information.

## Layout targets

- 1920x1080 desktop/full-game.
- 1280x720 desktop/windowed and handheld landscape.
- narrow portrait as a lower-priority sanity check.
- No outer-window scrolling. Dense panels own their scrolling.

## Functional target

- Controls mutate local demo state.
- Dropdowns open and commit a choice; they do not merely cycle.
- Sliders can be dragged and keyboard/gamepad adjusted where the toolkit
  supports it.
- A user can switch every destination without restarting the app.
- Native gamepad navigation is noted and tested, but custom spatial graph
  polish is not the primary discriminator in this round.

## Measurement target

- Release build, fixed populated state, 1280x720 and 1920x1080.
- Report UI update/build CPU time, draw submission CPU time when separable,
  complete frame CPU time, idle RSS, and binary size.
- The working budget is under 3 ms of UI CPU at 144 Hz, with substantially less
  than 1 ms preferred for state/update work.
