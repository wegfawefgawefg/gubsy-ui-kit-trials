export const initialState = {
  activeProfile: 'Moss',
  bindProfile: 'Default Binds',
  profiles: [
    { id: 1, name: 'Moss', played: '38h 22m', color: '#a9f08b', stats: { runs: 84, wins: 11, deaths: 73, deepest: 'Temple 7-3', score: '1,284,600', currency: '8,420', replays: 19 } },
    { id: 2, name: 'Vega', played: '14h 11m', color: '#79d7ff', stats: { runs: 31, wins: 4, deaths: 27, deepest: 'Ice 4-2', score: '488,120', currency: '2,115', replays: 7 } },
    { id: 3, name: 'Guest', played: 'New', color: '#ffc76e', stats: { runs: 0, wins: 0, deaths: 0, deepest: '—', score: '0', currency: '0', replays: 0 } },
  ],
  players: [
    { id: 1, name: 'Moss', profile: 'Moss', binds: 'Default Binds', input: 'Standard', device: 'Xbox Wireless Controller', local: true, ready: true },
    { id: 2, name: 'Open Slot', profile: 'Guest', binds: 'Default Binds', input: 'Standard', device: 'Press A to join', local: true, ready: false },
  ],
  devices: ['Xbox Wireless Controller', 'Keyboard + Mouse', 'DualSense Wireless Controller'],
  inputDevices: [
    { id: 'keyboard-main', name: 'Keyboard + Mouse', type: 'Keyboard / pointer', connection: 'Built in', assignedPlayerIds: [1], inputs: [
      { label: 'Keyboard W', kind: 'Button', value: 'UP' }, { label: 'Keyboard Space', kind: 'Button', value: 'UP' }, { label: 'Mouse Left', kind: 'Button', value: 'UP' }, { label: 'Mouse X/Y', kind: '2D axis', value: '+0.00, −0.01' }, { label: 'Mouse Wheel', kind: '1D axis', value: '0.00' },
    ] },
    { id: 'xbox-main', name: 'Xbox Wireless Controller', type: 'Gamepad', connection: 'Bluetooth · SDL gamepad 0', assignedPlayerIds: [1], inputs: [
      { label: 'A / South', kind: 'Button', value: 'UP' }, { label: 'B / East', kind: 'Button', value: 'UP' }, { label: 'Left Stick', kind: '2D axis', value: '+0.04, −0.02' }, { label: 'Left Trigger', kind: '1D axis', value: '0.00' }, { label: 'Right Trigger', kind: '1D axis', value: '0.18' }, { label: 'D-Pad Up', kind: 'Button', value: 'UP' },
    ] },
    { id: 'wheel-pedals', name: 'G29 Wheel + Pedals', type: 'Joystick', connection: 'USB · SDL joystick 1', assignedPlayerIds: [], inputs: [
      { label: 'Axis 0 · Steering', kind: '1D axis', value: '−0.07' }, { label: 'Axis 1 · Accelerator', kind: '1D axis', value: '0.34' }, { label: 'Axis 2 · Brake', kind: '1D axis', value: '0.00' }, { label: 'Button 0', kind: 'Button', value: 'UP' }, { label: 'Hat 0 Up', kind: 'Button', value: 'UP' },
    ] },
    { id: 'hotas-stick', name: 'T.16000M Flight Stick', type: 'Joystick', connection: 'USB · SDL joystick 2', assignedPlayerIds: [], inputs: [
      { label: 'Axes 0/1 · Stick', kind: '2D axis', value: '+0.01, +0.03' }, { label: 'Axis 2 · Twist', kind: '1D axis', value: '−0.02' }, { label: 'Axis 3 · Throttle', kind: '1D axis', value: '0.62' }, { label: 'Button 0 · Trigger', kind: 'Button', value: 'UP' }, { label: 'Hat 0 Right', kind: 'Button', value: 'DOWN' },
    ] },
    { id: 'macro-pad', name: 'Vega Macro Pad', type: 'Keyboard-like HID', connection: 'USB · keyboard 1', assignedPlayerIds: [1], inputs: [
      { label: 'Key F13', kind: 'Button', value: 'UP' }, { label: 'Key F14', kind: 'Button', value: 'UP' }, { label: 'Encoder clockwise', kind: 'Button', value: 'UP' }, { label: 'Encoder counter-clockwise', kind: 'Button', value: 'UP' },
    ] },
  ],
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
    'Run': ['Gamepad B', 'Left Bumper', 'Right Bumper'],
    'Run Trigger': ['Left Trigger · threshold 35%'],
    'Attack': ['Right Trigger → pressed at 35%', 'Mouse Left'],
    'Interact': ['Gamepad X', 'Keyboard E'],
    'Pause': ['Gamepad Menu', 'Keyboard Escape'],
  },
  saves: [
    { id: 1, name: 'The Glass Caverns', owner: 'Moss', kind: 'Campaign', detail: 'Temple Depths · 42%', time: '12h 48m', status: 'ready', stamp: 'Today, 06:52', modManifest: [{ name: 'Base Content', version: '1.0.0' }, { name: 'Cartographer’s Desk', version: '3.0.2' }], checkpoints: [{ name: 'Temple safe room', stamp: 'Today, 06:52', current: true }, { name: 'Flooded archive', stamp: 'Today, 06:18' }, { name: 'Mushroom crossing', stamp: 'Yesterday, 22:41' }] },
    { id: 2, name: 'A Quiet Beginning', owner: 'Vega', kind: 'Campaign', detail: 'Green Valley · 11%', time: '3h 06m', status: 'ready', stamp: 'Yesterday', modManifest: [{ name: 'Base Content', version: '1.0.0' }], checkpoints: [{ name: 'Valley camp', stamp: 'Yesterday, 18:20', current: true }, { name: 'Old well', stamp: 'Yesterday, 17:04' }] },
    { id: 3, name: 'Old Expedition', owner: 'Moss', kind: 'Legacy campaign', detail: 'Version 0.7 data', time: '8h 19m', status: 'incompatible', stamp: 'Aug 14', modManifest: [{ name: 'Base Content', version: '0.7.0' }, { name: 'Old Lanterns', version: '0.5.0' }], checkpoints: [{ name: 'Legacy checkpoint', stamp: 'Aug 14', current: true }] },
  ],
  mods: [
    { id: 1, name: 'Base Content', author: 'Gubsy', version: '1.0.0', versions: ['1.0.0'], enabled: true, sessionEnabled: true, required: true, runtimePolicy: 'new-session', dependencies: [], status: 'Compatible', sheet: 1, frame: 0, description: 'Core Splonks definitions and assets required by every session.' },
    { id: 2, name: 'Cartographer’s Desk', author: 'Yarrow', version: '3.0.2', versions: ['3.0.2','2.9.4','2.8.1'], enabled: true, sessionEnabled: true, runtimePolicy: 'hot-safe', dependencies: ['Base Content'], status: 'Compatible', sheet: 3, frame: 2, description: 'Shared map markers, route notes, unexplored-room hints, and color-coded party pings.' },
    { id: 3, name: 'Pocket Expedition', author: 'Mossbyte', version: '0.9.1', versions: ['0.9.1','0.8.7'], enabled: false, sessionEnabled: false, runtimePolicy: 'next-stage', dependencies: ['Base Content'], status: 'Compatible', sheet: 3, frame: 3, description: 'A complete toy-scale visual conversion with miniature equipment and diorama props.' },
    { id: 4, name: 'Old Lanterns', author: 'Blue Finch', version: '0.5.0', latestVersion: '1.1.0', versions: ['1.1.0','0.5.0'], enabled: false, sessionEnabled: false, runtimePolicy: 'hot-safe', dependencies: ['Base Content'], status: 'Needs update', sheet: 1, frame: 4, description: 'An archived lantern palette retained for older progression manifests.' },
  ],
  modCatalog: [
    { id: 101, name: 'Mycelium Below', author: 'Blue Finch', version: '2.3.1', rating: '96%', size: '84 MB', downloads: '18.4k', updated: '2 days ago', category: 'Worlds', tags: ['biomes','co-op'], sheet: 1, frame: 0, description: 'A hand-built bioluminescent biome with fungal rivers, spore weather, and twelve new encounter rooms.' },
    { id: 102, name: 'Brassline Grapple Kit', author: 'Cicada Works', version: '1.8.0', rating: '93%', size: '22 MB', downloads: '31.2k', updated: '1 week ago', category: 'Gameplay', tags: ['movement','gear'], sheet: 1, frame: 1, description: 'Adds a momentum-driven grappling hook, rope anchors, and traversal trials balanced for solo and cooperative runs.' },
    { id: 103, name: 'Skybreak Caverns', author: 'Yarrow', version: '1.2.4', rating: '91%', size: '126 MB', downloads: '9.7k', updated: '4 days ago', category: 'Worlds', tags: ['campaign','vertical'], dependencies: ['Brassline Grapple Kit'], sheet: 1, frame: 2, description: 'An alternate floating-island campaign where wind, falling debris, and unstable bridges reshape every route.' },
    { id: 104, name: 'Archive of Echoes', author: 'Palimpsest', version: '3.0.0', rating: '98%', size: '65 MB', downloads: '44.8k', updated: 'Yesterday', category: 'Worlds', tags: ['story','puzzles'], dependencies: ['Cartographer’s Desk'], sheet: 1, frame: 3, description: 'Explore a buried library whose rooms remember previous expeditions and turn old player choices into new puzzles.' },
    { id: 105, name: 'Lanternwake Festival', author: 'Marrowlight', version: '2.1.0', rating: '89%', size: '42 MB', downloads: '12.1k', updated: '3 weeks ago', category: 'Cosmetic', tags: ['event','decor'], dependencies: ['Base Content'], sheet: 1, frame: 4, description: 'Transforms safe settlements into warm lantern festivals with new music, decorations, food stalls, and photo props.' },
    { id: 106, name: 'Crystal Winter', author: 'Northglass', version: '1.4.7', rating: '94%', size: '97 MB', downloads: '21.3k', updated: '5 days ago', category: 'Worlds', tags: ['ice','hazards'], sheet: 2, frame: 0, description: 'A frozen cavern set with cracking ice, reflective crystal chambers, blizzards, and temperature-management rules.' },
    { id: 107, name: 'Clockwork Delve', author: 'Ternary Mine Co.', version: '0.9.8', rating: '87%', size: '58 MB', downloads: '7.6k', updated: '2 weeks ago', category: 'Worlds', tags: ['machines','traps'], dependencies: ['Brassline Grapple Kit'], sheet: 2, frame: 1, installable: false, compatibility: 'Requires Splonks 1.3 or newer', description: 'Descend through an abandoned automated mine packed with conveyor puzzles, crushers, lifts, and repairable machines.' },
    { id: 108, name: 'Ember Shrine', author: 'Red Mantle', version: '1.1.2', rating: '92%', size: '73 MB', downloads: '15.9k', updated: '6 days ago', category: 'Worlds', tags: ['lava','bosses'], sheet: 2, frame: 2, description: 'A volcanic temple campaign featuring rising lava, heat-reactive doors, ash storms, and two multi-stage guardians.' },
    { id: 109, name: 'Underground Rivers', author: 'Mossbyte', version: '2.6.5', rating: '95%', size: '49 MB', downloads: '37.0k', updated: 'Today', category: 'Gameplay', tags: ['water','boats'], dependencies: ['Base Content'], optionalDependencies: ['Mycelium Below'], sheet: 2, frame: 3, description: 'Introduces flowing water simulation, rafts, flooded shortcuts, river wildlife, and current-based cooperative puzzles.' },
    { id: 110, name: 'The Empty Camp', author: 'Quiet Signal', version: '1.0.3', rating: '90%', size: '36 MB', downloads: '8.2k', updated: '9 days ago', category: 'Story', tags: ['mystery','horror'], sheet: 2, frame: 4, installable: false, compatibility: 'Dependency “Signal Core” is unavailable', description: 'Follow abandoned campsites and damaged radio logs through a restrained mystery campaign with no combat additions.' },
    { id: 111, name: 'Verdant Ruins', author: 'Soft Stone', version: '4.2.0', rating: '97%', size: '118 MB', downloads: '52.4k', updated: '3 days ago', category: 'Worlds', tags: ['jungle','campaign'], dependencies: ['Base Content'], sheet: 3, frame: 0, description: 'A sprawling overgrown ruin set with living vines, seasonal chambers, hidden temples, and branching expedition goals.' },
    { id: 112, name: 'Sporefolk Village', author: 'Cap & Stem', version: '1.7.9', rating: '88%', size: '31 MB', downloads: '14.6k', updated: '1 month ago', category: 'Story', tags: ['town','quests'], dependencies: ['Mycelium Below'], sheet: 3, frame: 1, description: 'Adds a persistent mushroom settlement, resident questlines, merchants, and a village that changes across successful runs.' },
    { id: 113, name: 'Cartographer’s Desk', author: 'Yarrow', version: '3.0.2', rating: '97%', size: '7 MB', downloads: '63.1k', updated: 'Today', category: 'Tools', tags: ['maps','co-op'], dependencies: ['Base Content'], sheet: 3, frame: 2, description: 'Shared map markers, route notes, unexplored-room hints, and color-coded party pings without revealing hidden terrain.' },
    { id: 114, name: 'Pocket Expedition', author: 'Mossbyte', version: '0.9.1', rating: '86%', size: '28 MB', downloads: '11.0k', updated: '2 months ago', category: 'Cosmetic', tags: ['miniatures','skins'], dependencies: ['Base Content'], sheet: 3, frame: 3, description: 'A complete toy-scale visual conversion with miniature equipment, diorama props, portraits, and matching sound effects.' },
    { id: 115, name: 'Mirror Depths', author: 'Parallax Union', version: '1.3.6', rating: '94%', size: '76 MB', downloads: '19.8k', updated: '1 week ago', category: 'Gameplay', tags: ['puzzles','dimensions'], dependencies: ['Archive of Echoes'], sheet: 3, frame: 4, installable: false, compatibility: 'Requires Gubsy Mod API 0.2', description: 'Pairs every room with a reflected twin; party members shift between worlds to redirect hazards and open paths.' },
    { id: 116, name: 'Abyssal Tide', author: 'Bathysphere', version: '2.0.1', rating: '93%', size: '132 MB', downloads: '17.2k', updated: '4 days ago', category: 'Worlds', tags: ['underwater','survival'], dependencies: ['Underground Rivers'], sheet: 4, frame: 0, description: 'Flooded deep-sea caverns with air pockets, pressure hazards, luminous wildlife, diving gear, and submarine safe rooms.' },
    { id: 117, name: 'Sandburied Sanctum', author: 'Dune Lark', version: '1.5.3', rating: '90%', size: '88 MB', downloads: '13.5k', updated: '12 days ago', category: 'Worlds', tags: ['desert','temple'], sheet: 4, frame: 1, description: 'Excavate a shifting desert temple where sand fills chambers, uncovers relics, and permanently alters expedition routes.' },
    { id: 118, name: 'Alchemist’s Cache', author: 'Copper Flask', version: '2.2.8', rating: '92%', size: '19 MB', downloads: '29.7k', updated: '6 days ago', category: 'Items', tags: ['crafting','potions'], dependencies: ['Base Content'], optionalDependencies: ['Sporefolk Village'], sheet: 4, frame: 2, description: 'Adds carefully balanced field alchemy, discoverable formulas, volatile mixtures, and cooperative potion-combining.' },
    { id: 119, name: 'Wisp Ecology', author: 'Pale Orchard', version: '1.0.6', rating: '89%', size: '44 MB', downloads: '10.4k', updated: '3 weeks ago', category: 'Creatures', tags: ['wildlife','ambient'], sheet: 4, frame: 3, installable: false, compatibility: 'Not available for this renderer backend', description: 'Spectral wildlife migrates through the cave network, reacting to light, noise, weather, and the party’s past behavior.' },
    { id: 120, name: 'The Long Descent', author: 'Rope Team Six', version: '3.4.1', rating: '96%', size: '102 MB', downloads: '40.5k', updated: 'Yesterday', category: 'Challenge', tags: ['vertical','hard'], dependencies: ['Brassline Grapple Kit','Cartographer’s Desk'], sheet: 4, frame: 4, description: 'A continuous vertical megacave with persistent rope routes, collapsing bridges, scarce camps, and weekly seeded challenges.' },
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
  { id: 'settings', label: 'Settings', icon: '⚙', hint: 'Game preferences' },
  { id: 'controls', label: 'Controls', icon: '⌁', hint: 'Bindings & input' },
  { id: 'saves', label: 'Progress', icon: '▣', hint: 'Campaigns & checkpoints' },
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
