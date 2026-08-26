export const initialState = {
  activeProfile: 'Moss',
  bindProfile: 'Default Binds',
  profiles: [
    { id: 1, name: 'Moss', played: '12h 48m', color: '#a9f08b' },
    { id: 2, name: 'Vega', played: '4h 11m', color: '#79d7ff' },
    { id: 3, name: 'Guest', played: 'New', color: '#ffc76e' },
  ],
  players: [
    { id: 1, name: 'Moss', profile: 'Moss', binds: 'Default Binds', input: 'Standard', device: 'Xbox Wireless Controller', local: true, ready: true },
    { id: 2, name: 'Open Slot', profile: 'Guest', binds: 'Default Binds', input: 'Standard', device: 'Press A to join', local: true, ready: false },
  ],
  devices: ['Xbox Wireless Controller', 'Keyboard + Mouse', 'DualSense Wireless Controller'],
  bindProfiles: [
    { id: 1, name: 'Default Binds', owner: 'Moss' },
    { id: 2, name: 'Southpaw', owner: 'Vega' },
  ],
  settings: {
    fullscreen: true, resolution: '1920 × 1080', renderScale: '100%', frameCap: '144 FPS',
    brightness: 64, master: 82, music: 55, effects: 90, voice: 78,
    subtitles: true, textScale: 'Large', colorMode: 'Standard', shake: 35,
    sensitivity: 45, invertY: false, vibration: 80, deadzone: 12,
    difficulty: 'Standard', hints: true, autosave: true,
  },
  bindings: {
    'Menu Up': ['D-Pad Up', 'Keyboard W', 'Keyboard ↑'],
    'Menu Down': ['D-Pad Down', 'Keyboard S', 'Keyboard ↓'],
    'Menu Left': ['D-Pad Left', 'Keyboard A', 'Keyboard ←'],
    'Menu Right': ['D-Pad Right', 'Keyboard D', 'Keyboard →'],
    'Activate': ['Gamepad A', 'Keyboard Enter', 'Mouse Left'],
    'Cancel': ['Gamepad B', 'Keyboard Escape'],
    'Move': ['Left Stick', 'Keyboard WASD'],
    'Look': ['Right Stick', 'Mouse XY'],
    'Jump': ['Gamepad A', 'Keyboard Space'],
    'Attack': ['Right Trigger', 'Mouse Left'],
    'Interact': ['Gamepad X', 'Keyboard E'],
    'Pause': ['Gamepad Menu', 'Keyboard Escape'],
  },
  saves: [
    { id: 1, name: 'The Glass Caverns', detail: 'Temple Depths · 42%', time: '12h 48m', status: 'ready', stamp: 'Today, 06:52' },
    { id: 2, name: 'A Quiet Beginning', detail: 'Green Valley · 11%', time: '3h 06m', status: 'ready', stamp: 'Yesterday' },
    { id: 3, name: 'Old Expedition', detail: 'Version 0.7 data', time: '8h 19m', status: 'incompatible', stamp: 'Aug 14' },
  ],
  mods: [
    { id: 1, name: 'Cavern Cartographer', author: 'Yarrow', version: '1.4.2', enabled: true, status: 'Compatible', description: 'Adds annotated maps and discovered-path overlays.' },
    { id: 2, name: 'Small Splonks', author: 'Mossbyte', version: '0.9.1', enabled: false, status: 'Compatible', description: 'A tiny character pack with matching portraits.' },
    { id: 3, name: 'Old Lanterns', author: 'Blue Finch', version: '0.5.0', enabled: false, status: 'Needs update', description: 'Warm dynamic lantern palettes for cave stages.' },
  ],
  rooms: [
    { id: 1, name: 'Cave Club', host: 'Fern', players: '2 / 4', ping: 34, region: 'US East', status: 'Open' },
    { id: 2, name: 'After Work Splonks', host: 'Mina', players: '3 / 4', ping: 71, region: 'US West', status: 'Open' },
    { id: 3, name: 'No Spoilers', host: 'Quill', players: '4 / 4', ping: 112, region: 'EU West', status: 'Full' },
  ],
}

export const navItems = [
  { id: 'play', label: 'Play', icon: '▶', hint: 'Continue or start' },
  { id: 'players', label: 'Players', icon: '◆', hint: 'Profiles & devices' },
  { id: 'online', label: 'Online', icon: '◎', hint: 'Host or join' },
  { id: 'settings', label: 'Settings', icon: '⚙', hint: 'Game preferences' },
  { id: 'controls', label: 'Controls', icon: '⌁', hint: 'Bindings & input' },
  { id: 'saves', label: 'Saves', icon: '▣', hint: 'Continue & manage' },
  { id: 'mods', label: 'Mods', icon: '⬡', hint: 'Installed content' },
]

export const settingGroups = {
  Display: [
    { key: 'fullscreen', label: 'Fullscreen', note: 'Use the entire selected display.', type: 'toggle' },
    { key: 'resolution', label: 'Display resolution', note: 'Output resolution for this display.', type: 'select', options: ['1280 × 720', '1920 × 1080', '2560 × 1440'] },
    { key: 'renderScale', label: 'Render scale', note: 'Internal 3D resolution. UI remains native.', type: 'select', options: ['75%', '100%', '125%'] },
    { key: 'frameCap', label: 'Frame cap', note: 'Maximum rendered frames per second.', type: 'select', options: ['60 FPS', '120 FPS', '144 FPS', 'Unlimited'] },
    { key: 'brightness', label: 'Brightness', note: 'Tune shadow visibility.', type: 'range', min: 20, max: 100 },
  ],
  Audio: [
    { key: 'master', label: 'Master volume', note: 'All game audio.', type: 'range' },
    { key: 'music', label: 'Music', note: 'Exploration and menu music.', type: 'range' },
    { key: 'effects', label: 'Effects', note: 'World, movement, and combat.', type: 'range' },
    { key: 'voice', label: 'Voice', note: 'Character and narration volume.', type: 'range' },
  ],
  Accessibility: [
    { key: 'subtitles', label: 'Subtitles', note: 'Show spoken dialogue as text.', type: 'toggle' },
    { key: 'textScale', label: 'Text size', note: 'Scale menu and subtitle text.', type: 'select', options: ['Small', 'Standard', 'Large', 'Extra large'] },
    { key: 'colorMode', label: 'Color assistance', note: 'Adjust critical color pairs.', type: 'select', options: ['Standard', 'Deuteranopia', 'Protanopia', 'Tritanopia'] },
    { key: 'shake', label: 'Camera shake', note: 'Intensity of impact-driven camera movement.', type: 'range', min: 0, max: 100 },
  ],
  Gameplay: [
    { key: 'difficulty', label: 'Challenge', note: 'Enemy behavior and resource pressure.', type: 'select', options: ['Relaxed', 'Standard', 'Expedition'] },
    { key: 'hints', label: 'Context hints', note: 'Show reminders for unfamiliar mechanics.', type: 'toggle' },
    { key: 'autosave', label: 'Autosave', note: 'Save at safe rooms and chapter changes.', type: 'toggle' },
  ],
}
