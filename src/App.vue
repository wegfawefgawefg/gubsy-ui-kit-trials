<script setup>
import { computed, nextTick, onMounted, onUnmounted, reactive, ref, watch } from 'vue'
import { initialState, navItems, settingGroups } from './demoData'

const clone = (value) => JSON.parse(JSON.stringify(value))
const saved = localStorage.getItem('gubsy-ui-demo-state')
const state = reactive(saved ? { ...clone(initialState), ...JSON.parse(saved) } : clone(initialState))

// Forward-migrate browser-local demo data as the prototype model evolves.
state.profiles.forEach((profile) => {
  const sample = initialState.profiles.find((item) => item.name === profile.name) || initialState.profiles[2]
  profile.stats = { ...clone(sample.stats), ...(profile.stats || {}) }
})
state.saves.forEach((save) => {
  save.owner ||= state.activeProfile
  save.kind ||= 'Campaign'
  save.checkpoints ||= [{ name: 'Latest checkpoint', stamp: save.stamp, current: true }]
  save.modManifest ||= [{ name: 'Base Content', version: '1.0.0' }]
})
if (!state.mods.some((mod) => mod.name === 'Base Content')) state.mods = clone(initialState.mods)
state.mods.forEach((mod) => {
  const sample = initialState.mods.find((item) => item.name === mod.name)
  if (sample) {
    mod.sheet ||= sample.sheet; mod.frame ??= sample.frame
    mod.latestVersion ||= sample.latestVersion
    mod.runtimePolicy ||= sample.runtimePolicy
    if (sample.latestVersion && !(mod.versions || []).includes(sample.latestVersion)) mod.versions = [sample.latestVersion, ...(mod.versions || [mod.version])]
  }
  if (mod.sessionEnabled === undefined) mod.sessionEnabled = Boolean(mod.enabled)
  mod.dependencies ||= []
  mod.optionalDependencies ||= []
  mod.versions ||= [mod.version]
})
if (!state.modCatalog || state.modCatalog.length < 20) state.modCatalog = clone(initialState.modCatalog)
else state.modCatalog = state.modCatalog.map((mod) => ({ ...mod, ...clone(initialState.modCatalog.find((sample) => sample.id === mod.id) || {}) }))
state.modCatalog.forEach((mod) => { mod.dependencies ||= []; mod.optionalDependencies ||= []; mod.versions ||= [mod.version] })
if (!state.inputDevices) state.inputDevices = clone(initialState.inputDevices)
else state.inputDevices = initialState.inputDevices.map((sample) => ({ ...clone(sample), ...(state.inputDevices.find((device) => device.id === sample.id) || {}) }))
for (const [action, bindings] of Object.entries(initialState.bindings)) state.bindings[action] ||= clone(bindings)

const destination = ref('play')
const playerTab = ref('Local players')
const onlineTab = ref('Lobby') // retained for the archived comparison screen; no top-level route exposes it
const settingsTab = ref('Display')
const controlTab = ref('Bindings')
const saveTab = ref('Continue')
const modsTab = ref('Installed')
const sessionAccess = ref('Friends can join')
const sessionHost = ref('Automatic')
const sessionMode = ref('Continue expedition')
const selectedQuestId = ref('violet-reach')
const selectedCheckpoint = ref(state.saves[0]?.checkpoints?.[0]?.name)
const selectedRuleKey = ref('difficulty')
const selectedPlayer = ref(0)
const selectedAction = ref('Menu Up')
const selectedInputDeviceId = ref(state.inputDevices[0]?.id)
const inputPickerSearch = ref('')
const selectedSave = ref(state.saves[0]?.id)
const selectedMod = ref(state.mods[0]?.id)
const selectedCatalogMod = ref(state.modCatalog?.[0]?.id)
const selectedRoom = ref(state.rooms[0]?.id)
const scenario = ref('Populated')
const onlineStatus = ref('Offline')
const modal = ref(null)
const toast = ref('')
const inputMode = ref('controller')
const capture = ref(null)
const search = ref('')
const compatibleModsOnly = ref(false)
const playView = ref('lobby')
const sessionModsTab = ref('Current set')
const sessionModSelection = ref(state.mods[0]?.id)
const sessionCatalogSelection = ref(state.modCatalog?.[0]?.id)
const sessionModSearch = ref('')
const sessionRunning = ref(false)
const mobilePlayPane = ref('setup')
const mobileModDetail = ref(false)
const mobileLibraryDetail = ref(false)
const mobileQuestDetail = ref(false)
const mobileRuleDetail = ref(false)
let toastTimer

const bindingActionMeta = {
  'Menu Up': { kind: 'Digital action', category: 'Menu', output: 'Boolean' },
  'Menu Down': { kind: 'Digital action', category: 'Menu', output: 'Boolean' },
  'Menu Left': { kind: 'Digital action', category: 'Menu', output: 'Boolean' },
  'Menu Right': { kind: 'Digital action', category: 'Menu', output: 'Boolean' },
  Activate: { kind: 'Digital action', category: 'Menu', output: 'Boolean' },
  Cancel: { kind: 'Digital action', category: 'Menu', output: 'Boolean' },
  Move: { kind: '2D analog output', category: 'Gameplay', output: '2D axis' },
  Look: { kind: '2D analog output', category: 'Gameplay', output: '2D axis' },
  Jump: { kind: 'Digital action', category: 'Gameplay', output: 'Boolean' },
  Run: { kind: 'Digital action', category: 'Gameplay · Run group', output: 'Boolean' },
  'Run Trigger': { kind: '1D analog output', category: 'Gameplay · Run group', output: '1D axis' },
  Attack: { kind: 'Digital action', category: 'Gameplay', output: 'Boolean' },
  Interact: { kind: 'Digital action', category: 'Gameplay', output: 'Boolean' },
  Pause: { kind: 'Digital action', category: 'Menu / Gameplay', output: 'Boolean' },
}

const quests = [
  { id: 'violet-reach', name: 'The Violet Reach', region: 'Temple frontier', length: '6 stages', difficulty: 'Moderate', description: 'Follow a fractured relay signal through flooded archives, fungal crossings, and a temple complex waking beneath the mountain.', stages: ['North Pass', 'Mushroom Crossing', 'Flooded Archive', 'Temple Gate', 'The Relay', 'Violet Core'] },
  { id: 'sunken-road', name: 'The Sunken Road', region: 'River caverns', length: '5 stages', difficulty: 'Hard', description: 'Recover the brass survey team’s route through moving waterways where every opened sluice changes the return path.', stages: ['Old Well', 'Broken Aqueduct', 'Ferryman’s Lock', 'Drowned Market', 'River Heart'] },
  { id: 'green-below', name: 'A Green Beginning', region: 'Valley underworks', length: '4 stages', difficulty: 'Welcoming', description: 'A shorter introductory quest through farms, limestone tunnels, and the first settlements below the valley.', stages: ['Valley Camp', 'Root Cellar', 'Limestone Steps', 'Lantern Town'] },
  { id: 'glass-pilgrim', name: 'The Glass Pilgrim', region: 'Crystal descent', length: '7 stages', difficulty: 'Severe', description: 'Track a silent pilgrim into refracting chambers where false routes, light puzzles, and brittle floors punish haste.', stages: ['Shale Mouth', 'Prism Hall', 'Blue Fault', 'Mirror Camp', 'Crystal Choir', 'Lightless Span', 'Pilgrim’s Rest'] },
]
const sessionRules = reactive({ difficulty: 'Standard', generation: 'Quest-authored', lives: 4, health: 4, damage: 100, speed: 100, ghostTimer: 180, shops: 'Normal', shortcuts: true, sharedMoney: true, friendlyFire: false, revives: 'At next stage', rounds: 5, timeLimit: 180, bombs: 4, ropes: 4, pickups: 'Standard', mapRotation: 'All arenas', teams: 'Free for all', suddenDeath: true, spectators: true, sharedMap: true, markerLifetime: 45, lanternFuel: 'Standard' })
const expeditionRuleDefinitions = [
  { key: 'difficulty', label: 'Expedition difficulty', note: 'Overall danger and resource pressure.', detail: 'Adjusts enemy patterns, trap density, and the generosity of recovery rooms without changing the authored quest route.', type: 'select', options: ['Relaxed','Standard','Relentless'] },
  { key: 'generation', label: 'Stage variation', note: 'How authored rooms are remixed.', detail: 'Quest-authored preserves major landmarks. Remixed varies connecting rooms. Wild allows the broadest procedural substitutions.', type: 'select', options: ['Quest-authored','Remixed','Wild'] },
  { key: 'lives', label: 'Shared lives', note: 'Continues available to the party.', detail: 'When the party wipes, one shared life restarts the current stage. At zero, the expedition returns to its last persistent checkpoint.', type: 'range', min: 0, max: 9, unit: '' },
  { key: 'health', label: 'Starting health', note: 'Hearts granted at expedition start.', detail: 'Applies to each joining player. Continuing players keep their checkpoint health unless recovery-on-resume is enabled.', type: 'range', min: 1, max: 12, unit: ' hearts' },
  { key: 'ghostTimer', label: 'Ghost arrival', note: 'Seconds before the stage begins hunting.', detail: 'The pursuit ghost enters after this time. Longer timers encourage exploration; shorter timers force decisive routes.', type: 'range', min: 60, max: 300, step: 30, unit: ' sec' },
  { key: 'shops', label: 'Shop frequency', note: 'Merchant room availability.', detail: 'Controls optional shop placements between required quest landmarks. Inventory rolls remain governed by the quest.', type: 'select', options: ['Sparse','Normal','Frequent'] },
  { key: 'shortcuts', label: 'Discovered shortcuts', note: 'Allow unlocked route entrances.', detail: 'Lets the party enter from previously discovered quest junctions. Disabled for a clean route from stage one.', type: 'toggle' },
  { key: 'sharedMoney', label: 'Shared treasury', note: 'Combine party currency.', detail: 'All collected currency enters one pool used by any player. Disable for individual wallets and purchases.', type: 'toggle' },
  { key: 'friendlyFire', label: 'Friendly fire', note: 'Party attacks can cause damage.', detail: 'Traps and thrown objects always interact with the party; this option additionally enables direct weapon damage.', type: 'toggle' },
  { key: 'revives', label: 'Defeated players return', note: 'Co-op recovery rule.', detail: 'Controls when defeated players can rejoin without consuming the entire party’s shared life.', type: 'select', options: ['At next room','At next stage','Only at checkpoints'] },
]
const arenaRuleDefinitions = [
  { key: 'teams', label: 'Team format', note: 'How victory and spawning are grouped.', detail: 'Free for all scores each player independently. Teams share score, spawn colors, and round victories.', type: 'select', options: ['Free for all','2 versus 2','Asymmetric hunt'] },
  { key: 'rounds', label: 'Rounds to win', note: 'Match victory threshold.', detail: 'The first player or team to win this many rounds takes the match. Tied final rounds enter sudden death when enabled.', type: 'range', min: 1, max: 15, unit: '' },
  { key: 'timeLimit', label: 'Round time', note: 'Maximum seconds per round.', detail: 'At expiry the surviving player with the most treasure wins; exact ties invoke sudden death.', type: 'range', min: 30, max: 600, step: 30, unit: ' sec' },
  { key: 'health', label: 'Starting health', note: 'Hearts granted each round.', detail: 'Every player respawns with this health at the beginning of a round.', type: 'range', min: 1, max: 12, unit: ' hearts' },
  { key: 'damage', label: 'Player damage', note: 'Global damage multiplier.', detail: 'Scales direct attacks, thrown objects, explosions, and environmental damage caused by another player.', type: 'range', min: 25, max: 300, step: 25, unit: '%' },
  { key: 'speed', label: 'Movement speed', note: 'Global player movement multiplier.', detail: 'Changes run, climb, and swim speed. Jump arcs are compensated so authored arena geometry remains reachable.', type: 'range', min: 50, max: 200, step: 10, unit: '%' },
  { key: 'bombs', label: 'Starting bombs', note: 'Bombs granted each round.', detail: 'Sets each player’s starting stock. Arena pickups may add more when enabled by the pickup set.', type: 'range', min: 0, max: 20, unit: '' },
  { key: 'ropes', label: 'Starting ropes', note: 'Ropes granted each round.', detail: 'Sets each player’s starting traversal stock. Some compact arenas ignore ropes entirely.', type: 'range', min: 0, max: 20, unit: '' },
  { key: 'pickups', label: 'Arena pickups', note: 'Items that can appear during play.', detail: 'Competitive removes high-variance items; Chaotic enables the complete item pool and shorter spawn intervals.', type: 'select', options: ['None','Competitive','Standard','Chaotic'] },
  { key: 'mapRotation', label: 'Map rotation', note: 'Arena pool used between rounds.', detail: 'Choose the curated standard pool, every installed arena, or a random subset weighted away from repeats.', type: 'select', options: ['Standard rotation','All arenas','Random five'] },
  { key: 'suddenDeath', label: 'Sudden death', note: 'Resolve tied rounds.', detail: 'Removes safe terrain over time until only one player or team remains.', type: 'toggle' },
  { key: 'spectators', label: 'Late-join spectators', note: 'Allow watching until next match.', detail: 'New remote players may enter immediately as spectators and join the roster after the current match.', type: 'toggle' },
]

const currentProfile = computed(() => state.profiles.find((item) => item.name === state.activeProfile) || state.profiles[0])
const currentPlayer = computed(() => state.players[selectedPlayer.value] || state.players[0])
const currentBindProfile = computed(() => state.bindProfiles.find((item) => item.name === state.bindProfile) || state.bindProfiles[0])
const currentSave = computed(() => state.saves.find((item) => item.id === selectedSave.value) || state.saves[0])
const selectedQuest = computed(() => quests.find((quest) => quest.id === selectedQuestId.value) || quests[0])
const lobbyPlayers = computed(() => {
  const actual = state.players.filter((player) => player.name !== 'Open Slot')
  return sessionAccess.value === 'Solo' ? actual.slice(0, 1) : actual
})
const lobbyCapacity = computed(() => sessionAccess.value === 'Solo' ? 1 : 4)
const currentMod = computed(() => state.mods.find((item) => item.id === selectedMod.value) || state.mods[0])
const currentCatalogMod = computed(() => state.modCatalog?.find((item) => item.id === selectedCatalogMod.value) || state.modCatalog?.[0])
const currentRoom = computed(() => state.rooms.find((item) => item.id === selectedRoom.value) || state.rooms[0])
const currentSessionMod = computed(() => state.mods.find((item) => item.id === sessionModSelection.value) || state.mods[0])
const currentSessionCatalogMod = computed(() => state.modCatalog.find((item) => item.id === sessionCatalogSelection.value) || state.modCatalog[0])
const filteredSessionCatalog = computed(() => state.modCatalog.filter((item) => `${item.name} ${item.author} ${item.category} ${(item.tags || []).join(' ')}`.toLowerCase().includes(sessionModSearch.value.toLowerCase())))
const savedModStatus = computed(() => {
  const manifest = currentSave.value?.modManifest || []
  const missing = manifest.filter((entry) => !state.mods.some((mod) => mod.name === entry.name && mod.version === entry.version))
  const inactive = manifest.filter((entry) => !state.mods.some((mod) => mod.name === entry.name && mod.version === entry.version && mod.sessionEnabled))
  const extras = state.mods.filter((mod) => mod.sessionEnabled && !mod.required && !manifest.some((entry) => entry.name === mod.name && entry.version === mod.version))
  return { manifest, missing, inactive, extras, exact: !missing.length && !inactive.length && !extras.length }
})
const activeSessionMods = computed(() => state.mods.filter((mod) => mod.sessionEnabled))
const modRuleDefinitions = computed(() => {
  const rules = []
  if (activeSessionMods.value.some((mod) => mod.name === 'Cartographer’s Desk')) rules.push(
    { key: 'sharedMap', label: 'Shared map discoveries', note: 'Reveal visited rooms to the whole party.', detail: 'Newly explored rooms and route notes are synchronized between party maps. Disabling keeps private exploration histories separate.', type: 'toggle', source: 'Cartographer’s Desk' },
    { key: 'markerLifetime', label: 'Party marker lifetime', note: 'Seconds before temporary map pings fade.', detail: 'Controls how long contextual party markers remain visible on the map and compass after they are placed.', type: 'range', min: 10, max: 120, step: 5, unit: ' sec', source: 'Cartographer’s Desk' },
  )
  if (activeSessionMods.value.some((mod) => mod.name === 'Old Lanterns')) rules.push(
    { key: 'lanternFuel', label: 'Lantern fuel consumption', note: 'How quickly restored lanterns burn fuel.', detail: 'Changes the drain rate for lantern items supplied by Old Lanterns. Existing lantern state is migrated immediately.', type: 'select', options: ['Off','Slow','Standard','Harsh'], source: 'Old Lanterns' },
  )
  return rules
})
const activeRuleDefinitions = computed(() => [...(sessionMode.value === 'Arena' ? arenaRuleDefinitions : expeditionRuleDefinitions), ...modRuleDefinitions.value])
const currentRuleDefinition = computed(() => activeRuleDefinitions.value.find((rule) => rule.key === selectedRuleKey.value) || activeRuleDefinitions.value[0])
const sessionContentSummary = computed(() => {
  if (sessionMode.value === 'Continue expedition') return `${activeSessionMods.value.length} active · ${savedModStatus.value.exact ? 'matches checkpoint' : 'differs from checkpoint'}`
  if (sessionMode.value === 'New expedition') return `${activeSessionMods.value.length} active · recorded when expedition begins`
  return `${activeSessionMods.value.length} active · required for this match`
})
const modalProfile = computed(() => modal.value?.type === 'profile-history' ? state.profiles.find((item) => item.id === modal.value.payload) : null)
const filteredBindings = computed(() => Object.entries(state.bindings).filter(([name]) => name.toLowerCase().includes(search.value.toLowerCase())))
const currentBindingMeta = computed(() => bindingActionMeta[selectedAction.value] || { kind: 'Digital action', category: 'Game', output: 'Boolean' })
const currentInputDevice = computed(() => state.inputDevices.find((device) => device.id === selectedInputDeviceId.value) || state.inputDevices[0])
const localAssignablePlayers = computed(() => state.players.filter((player) => player.local && player.name !== 'Open Slot'))
const manualInputChoices = computed(() => state.inputDevices.flatMap((device) => device.inputs.map((input) => ({ ...input, deviceId: device.id, deviceName: device.name }))).filter((input) => `${input.deviceName} ${input.label} ${input.kind}`.toLowerCase().includes(inputPickerSearch.value.toLowerCase())))
const filteredCatalog = computed(() => state.modCatalog.filter((item) => {
  const haystack = `${item.name} ${item.author} ${item.category} ${item.tags.join(' ')}`.toLowerCase()
  return haystack.includes(search.value.toLowerCase()) && (!compatibleModsOnly.value || item.installable !== false)
}))
const sectionTitle = computed(() => navItems.find((item) => item.id === destination.value)?.label || 'Menu')

watch(state, () => localStorage.setItem('gubsy-ui-demo-state', JSON.stringify(state)), { deep: true })
watch(sessionMode, () => { selectedRuleKey.value = activeRuleDefinitions.value[0].key })
watch(playView, () => { mobileModDetail.value = false; mobileQuestDetail.value = false; mobileRuleDetail.value = false })
watch(sessionModsTab, () => { mobileModDetail.value = false })
watch(modsTab, () => { mobileLibraryDetail.value = false })
watch(selectedSave, (id) => {
  selectedQuestId.value = id === 2 ? 'green-below' : id === 3 ? 'sunken-road' : 'violet-reach'
  selectedCheckpoint.value = currentSave.value?.checkpoints?.[0]?.name
})

function notify(message) {
  toast.value = message
  clearTimeout(toastTimer)
  toastTimer = setTimeout(() => { toast.value = '' }, 2600)
}

function resetSessionRules() {
  Object.assign(sessionRules, sessionMode.value === 'Arena'
    ? { teams: 'Free for all', rounds: 5, timeLimit: 180, health: 4, damage: 100, speed: 100, bombs: 4, ropes: 4, pickups: 'Standard', mapRotation: 'All arenas', suddenDeath: true, spectators: true }
    : { difficulty: 'Standard', generation: 'Quest-authored', lives: 4, health: 4, ghostTimer: 180, shops: 'Normal', shortcuts: true, sharedMoney: true, friendlyFire: false, revives: 'At next stage' })
  notify(`${sessionMode.value} defaults restored`)
}

function chooseDestination(id) {
  destination.value = id
  if (id === 'play') playView.value = 'lobby'
  mobilePlayPane.value = 'setup'
  search.value = ''
  nextTick(focusFirstContent)
}

function focusFirstContent() {
  document.querySelector('.content [data-focus]:not([disabled])')?.focus()
}

function showModal(type, title, options = {}) {
  modal.value = { type, title, value: options.value || '', message: options.message || '', payload: options.payload, danger: options.danger || false }
  nextTick(() => document.querySelector('.modal [data-focus]')?.focus())
}

function closeModal() {
  if (modal.value?.type === 'capture') capture.value = null
  modal.value = null
  nextTick(focusFirstContent)
}

function confirmModal() {
  const item = modal.value
  if (!item) return
  const name = item.value.trim()
  if (item.type === 'new-profile' && name) {
    state.profiles.push({ id: Date.now(), name, played: 'New', color: '#ffc76e' })
    state.activeProfile = name
    notify(`Created profile “${name}”`)
  } else if (item.type === 'rename-profile' && name) {
    const profile = state.profiles.find((entry) => entry.id === item.payload)
    if (profile) {
      const oldName = profile.name
      profile.name = name
      if (state.activeProfile === oldName) state.activeProfile = name
      state.players.forEach((player) => { if (player.profile === oldName) player.profile = name })
      notify('Profile renamed')
    }
  } else if (item.type === 'delete-profile') {
    state.profiles = state.profiles.filter((entry) => entry.id !== item.payload)
    state.activeProfile = state.profiles[0]?.name || ''
    notify('Profile deleted')
  } else if (item.type === 'reset-profile') {
    notify('Profile preferences reset; save progress preserved')
  } else if (item.type === 'new-bind-profile' && name) {
    state.bindProfiles.push({ id: Date.now(), name, owner: state.activeProfile })
    state.bindProfile = name
    notify('Binding profile created')
  } else if (item.type === 'rename-bind-profile' && name) {
    const profile = state.bindProfiles.find((entry) => entry.id === item.payload)
    if (profile) {
      const oldName = profile.name
      profile.name = name
      if (state.bindProfile === oldName) state.bindProfile = name
      state.players.forEach((player) => { if (player.binds === oldName) player.binds = name })
      notify('Binding profile renamed')
    }
  } else if (item.type === 'delete-bind-profile') {
    state.bindProfiles = state.bindProfiles.filter((entry) => entry.id !== item.payload)
    state.bindProfile = state.bindProfiles[0]?.name || ''
    notify('Binding profile deleted')
  } else if (item.type === 'reset-bind-profile') {
    state.bindings = clone(initialState.bindings)
    notify('Bindings restored to registered defaults')
  } else if (item.type === 'rename-save' && name) {
    const save = state.saves.find((entry) => entry.id === item.payload)
    if (save) save.name = name
    notify('Save renamed')
  } else if (item.type === 'delete-save') {
    state.saves = state.saves.filter((entry) => entry.id !== item.payload)
    selectedSave.value = state.saves[0]?.id
    notify('Save deleted')
  } else if (item.type === 'apply-display') {
    notify('Display mode applied; auto-revert timer simulated')
  } else if (item.type === 'quit') {
    notify('Quit command sent to the host (demo)')
  } else if (item.type === 'reset-demo') {
    Object.assign(state, clone(initialState))
    notify('Demo data restored')
  }
  closeModal()
}

function copySave(save) {
  const id = Date.now()
  state.saves.push({ ...clone(save), id, name: `${save.name} — Copy`, stamp: 'Just now' })
  selectedSave.value = id
  notify('Save copied')
}

function findMod(name) { return state.mods.find((mod) => mod.name === name) }
function findCatalogMod(name) { return state.modCatalog.find((mod) => mod.name === name) }
function dependencyRows(mod) {
  return [
    ...(mod?.dependencies || []).map((name) => ({ name, kind: 'Required', installed: Boolean(findMod(name)) })),
    ...(mod?.optionalDependencies || []).map((name) => ({ name, kind: 'Optional', installed: Boolean(findMod(name)) })),
  ]
}
function dependentMods(name) { return state.mods.filter((mod) => (mod.dependencies || []).includes(name)) }
function dependentClosure(name, seen = new Set()) {
  for (const mod of dependentMods(name)) if (!seen.has(mod.name)) { seen.add(mod.name); dependentClosure(mod.name, seen) }
  return [...seen]
}
function installCatalogMod(catalogMod, addToSession = false, quiet = false, seen = new Set()) {
  if (!catalogMod || seen.has(catalogMod.name)) return findMod(catalogMod?.name)
  if (catalogMod.installable === false) { if (!quiet) notify(catalogMod.compatibility); return null }
  if (sessionRunning.value && addToSession && !quiet) {
    const installed = installCatalogMod(catalogMod, false, true, seen)
    if (installed) toggleSessionMod(installed)
    return installed
  }
  seen.add(catalogMod.name)
  for (const name of catalogMod.dependencies || []) {
    if (!findMod(name)) {
      const dependency = findCatalogMod(name)
      if (!dependency || !installCatalogMod(dependency, addToSession, true, seen)) { if (!quiet) notify(`Cannot install: required dependency ${name} is unavailable`); return null }
    }
  }
  let mod = findMod(catalogMod.name)
  if (!mod) {
    mod = { ...clone(catalogMod), id: Date.now() + state.mods.length, enabled: false, sessionEnabled: addToSession, status: 'Compatible', versions: catalogMod.versions || [catalogMod.version] }
    state.mods.push(mod)
  } else if (addToSession) mod.sessionEnabled = true
  if (addToSession) for (const name of mod.dependencies || []) { const dependency = findMod(name); if (dependency) dependency.sessionEnabled = true }
  catalogMod.status = 'Installed'
  selectedMod.value = mod.id
  sessionModSelection.value = mod.id
  if (!quiet) notify(addToSession ? `${catalogMod.name} and required dependencies added to this lobby` : `${catalogMod.name} and required dependencies installed`)
  return mod
}
function runtimePolicy(mod, seen = new Set()) {
  if (!mod || seen.has(mod.name)) return 'hot-safe'
  seen.add(mod.name)
  const own = mod.runtimePolicy || (['Cosmetic', 'Tools'].includes(mod.category) ? 'hot-safe' : ['Items', 'Creatures', 'Gameplay'].includes(mod.category) ? 'next-stage' : 'new-session')
  const rank = { 'hot-safe': 0, 'next-stage': 1, 'new-session': 2 }
  return (mod.dependencies || []).map(findMod).filter((dependency) => dependency && !dependency.sessionEnabled).reduce((strictest, dependency) => {
    const dependencyPolicy = runtimePolicy(dependency, seen)
    return rank[dependencyPolicy] > rank[strictest] ? dependencyPolicy : strictest
  }, own)
}
function runtimePolicyLabel(mod) {
  return { 'hot-safe': 'Can change while playing', 'next-stage': 'Applies cleanly next stage', 'new-session': 'Designed for a new session' }[runtimePolicy(mod)]
}
function toggleSessionMod(mod, runtimeConfirmed = false) {
  if (mod.required) return notify('Base Content is required by the game')
  if (mod.status === 'Needs update') return updateMod(mod, true)
  if (mod.status !== 'Compatible') return notify('Update this mod before enabling it')
  if (sessionRunning.value && !runtimeConfirmed && runtimePolicy(mod) !== 'hot-safe') {
    return showModal('runtime-mod-change', mod.sessionEnabled ? `Remove ${mod.name} while playing?` : `Add ${mod.name} while playing?`, { payload: { id: mod.id, enable: !mod.sessionEnabled, policy: runtimePolicy(mod) }, message: runtimePolicy(mod) === 'next-stage' ? 'This content can be queued safely for the next stage. Applying it immediately may mean this stage has already missed its new items or encounters.' : 'This mod changes world or progression content and is designed for a fresh session. You can still queue it, but the current world will not be retroactively regenerated.' })
  }
  if (!mod.sessionEnabled) {
    for (const name of mod.dependencies || []) { const dependency = findMod(name); if (!dependency) return notify(`Missing required dependency: ${name}`); dependency.sessionEnabled = true }
    mod.sessionEnabled = true
    return notify(`${mod.name} enabled with its requirements`)
  }
  const affected = dependentClosure(mod.name).filter((name) => findMod(name)?.sessionEnabled)
  if (affected.length) return showModal('disable-chain', 'Disable dependent mods?', { payload: { root: mod.name, names: affected }, message: `${affected.join(', ')} require ${mod.name}. They must also be disabled.` })
  mod.sessionEnabled = false
}
function applyRuntimeModChange(action) {
  const item = modal.value
  const mod = state.mods.find((entry) => entry.id === item?.payload?.id)
  if (!mod) return closeModal()
  if (action === 'now') {
    toggleSessionMod(mod, true)
    notify(`${mod.name} ${item.payload.enable ? 'enabled' : 'disabled'} now; existing world state preserved`)
  } else if (action === 'clear') {
    toggleSessionMod(mod, true)
    notify(`${mod.name} disabled and its owned entities/state cleared`)
  } else {
    mod.pendingRuntime = item.payload.enable ? 'Enable next stage' : 'Disable next stage'
    notify(`${mod.name} queued for the next safe boundary`)
  }
  closeModal()
}
function updateMod(mod, enableAfter = false) {
  if (!mod.latestVersion) return notify(`No compatible update is available for ${mod.name}`)
  for (const name of mod.dependencies || []) {
    const dependency = findMod(name)
    if (!dependency) return notify(`Cannot update: missing required dependency ${name}`)
    if (enableAfter) dependency.sessionEnabled = true
  }
  mod.version = mod.latestVersion
  if (!mod.versions.includes(mod.latestVersion)) mod.versions.unshift(mod.latestVersion)
  mod.status = 'Compatible'
  if (enableAfter) mod.sessionEnabled = true
  notify(`${mod.name} updated to ${mod.version}${enableAfter ? ' and enabled' : ''}`)
}
function requestUninstall(mod) {
  if (mod.required) return notify('Base Content is part of the game and cannot be uninstalled')
  const names = [mod.name, ...dependentClosure(mod.name)]
  showModal('uninstall-chain', names.length > 1 ? 'Remove dependent mods?' : `Uninstall ${mod.name}?`, { payload: { names }, message: names.length > 1 ? `${names.slice(1).join(', ')} depend on ${mod.name}. Removing it also removes those mods; saved manifests are preserved so they can be restored later.` : 'Saved manifests are preserved so this version can be restored later.', danger: true })
}
function applyModChainAction() {
  const item = modal.value
  if (item?.type === 'disable-chain') {
    for (const name of [item.payload.root, ...item.payload.names]) { const mod = findMod(name); if (mod && !mod.required) mod.sessionEnabled = false }
    notify('Dependent mod set disabled')
  } else if (item?.type === 'uninstall-chain') {
    state.mods = state.mods.filter((mod) => !item.payload.names.includes(mod.name))
    selectedMod.value = state.mods[0]?.id; sessionModSelection.value = state.mods[0]?.id
    notify(`${item.payload.names.length} mod${item.payload.names.length === 1 ? '' : 's'} removed from this device`)
  }
  closeModal()
}
function syncModsToProgress() {
  const manifest = currentSave.value?.modManifest || []
  for (const entry of manifest) {
    let installed = findMod(entry.name)
    if (!installed) installed = installCatalogMod(findCatalogMod(entry.name), true, true)
    if (!installed) return notify(`Cannot restore ${entry.name} ${entry.version}`)
    if (!(installed.versions || [installed.version]).includes(entry.version)) return notify(`${entry.name} ${entry.version} is unavailable`)
    installed.version = entry.version; installed.sessionEnabled = true
  }
  state.mods.forEach((mod) => { if (!mod.required && !manifest.some((entry) => entry.name === mod.name)) mod.sessionEnabled = false })
  notify(`Lobby synchronized to ${currentSave.value.name}`)
}

function modCompatibility(mod) {
  return mod.installable === false ? mod.compatibility : 'Compatible with this build'
}

function toggleCompatibleMods() {
  compatibleModsOnly.value = !compatibleModsOnly.value
  if (compatibleModsOnly.value && currentCatalogMod.value?.installable === false) {
    selectedCatalogMod.value = state.modCatalog.find((mod) => mod.installable !== false)?.id
  }
}

function modThumbStyle(mod) {
  return {
    backgroundImage: `url('/mod-sheet-${mod.sheet || 1}.png')`,
    backgroundPosition: `${(mod.frame || 0) * 25}% center`,
  }
}

function addLocalPlayer() {
  if (state.players.length >= 4) return notify('The local roster is full')
  state.players.push({ id: Date.now(), name: `Player ${state.players.length + 1}`, profile: 'Guest', binds: 'Default Binds', input: 'Standard', device: 'Press A to join', local: true, ready: false })
  selectedPlayer.value = state.players.length - 1
  notify('Local player added')
}

function removeLocalPlayer() {
  if (state.players.length <= 1) return notify('At least one local player is required')
  state.players.splice(selectedPlayer.value, 1)
  selectedPlayer.value = Math.max(0, selectedPlayer.value - 1)
  notify('Local player removed')
}

function refreshDevices() {
  scenario.value = 'Loading'
  notify('Scanning for input devices…')
  setTimeout(() => {
    scenario.value = 'Populated'
    if (!state.devices.includes('8BitDo Pro 2')) state.devices.push('8BitDo Pro 2')
    notify('Found 4 input devices')
  }, 900)
}

function beginCapture(action, slot = null) {
  capture.value = { action, slot }
  showModal('capture', 'Listening for input', { message: `Move an axis or press a button for “${action}”. Escape cancels.` })
}

function beginManualBinding(action, slot = null) {
  inputPickerSearch.value = ''
  showModal('input-picker', `Choose input for ${action}`, { payload: { action, slot } })
}

function bindingLabelForInput(action, input) {
  const output = (bindingActionMeta[action] || {}).output || 'Boolean'
  if (output === 'Boolean' && input.kind.includes('axis')) return `${input.deviceName} · ${input.label} → pressed at 35%`
  if (output === '1D axis' && input.kind === 'Button') return `${input.deviceName} · ${input.label} → 1.0 while held`
  if (output === '2D axis' && input.kind === 'Button') return `${input.deviceName} · ${input.label} → digital composite`
  return `${input.deviceName} · ${input.label}`
}

function assignBinding(action, slot, label) {
  const list = state.bindings[action]
  if (slot === null || slot === undefined) list.push(label)
  else list[slot] = label
  selectedAction.value = action
  notify(`Bound ${label}`)
}

function chooseManualInput(input) {
  const payload = modal.value?.payload
  if (!payload) return
  assignBinding(payload.action, payload.slot, bindingLabelForInput(payload.action, input))
  closeModal()
}

function assignDevicePlayer(device, playerId) {
  device.assignedPlayerIds = playerId === null ? [] : [playerId]
  notify(playerId === null ? `${device.name} is unassigned` : `${device.name} assigned to ${state.players.find((player) => player.id === playerId)?.name}`)
}

function pulseDeviceInput(input) {
  const previous = input.value
  input.value = input.kind === 'Button' ? 'DOWN' : input.kind === '2D axis' ? '+0.72, −0.38' : '0.76'
  setTimeout(() => { input.value = previous }, 850)
}

function captureInput(event) {
  if (!capture.value || event.key === 'Escape') return
  event.preventDefault()
  const label = event.key.length === 1 ? `Keyboard ${event.key.toUpperCase()}` : `Keyboard ${event.key}`
  const { action, slot } = capture.value
  assignBinding(action, slot, label)
  capture.value = null
  modal.value = null
}

function removeBinding(action, index) {
  state.bindings[action].splice(index, 1)
  notify('Binding removed')
}

function startHosting(kind) {
  onlineStatus.value = 'Connecting'
  notify(`Starting ${kind} session…`)
  setTimeout(() => {
    onlineStatus.value = kind === 'local' ? 'Local lobby' : 'Hosting · Ready'
    notify(kind === 'local' ? 'Local lobby ready' : 'Room VIOLET-7 is live')
  }, 850)
}

function joinRoom(room) {
  if (room.status === 'Full') return notify('That room is full')
  onlineStatus.value = 'Connecting'
  setTimeout(() => {
    onlineStatus.value = `Joined · ${room.name}`
    closeModal()
    notify(`Joined ${room.name}`)
  }, 850)
}

function startSession() {
  if (sessionAccess.value === 'Solo') onlineStatus.value = 'Solo session'
  else if (sessionAccess.value === 'Local players only') onlineStatus.value = 'Local session'
  else onlineStatus.value = `${sessionAccess.value} · ${sessionHost.value}`
  sessionRunning.value = true
  notify(`${sessionMode.value} started`)
}

async function copyInviteLink() {
  const link = 'splonks://join/session-7f3a'
  try { await navigator.clipboard.writeText(link) } catch { /* browser may deny clipboard in demo */ }
  notify('Session invite link copied')
}

function setScenario(value) {
  scenario.value = value
  if (value === 'Loading') setTimeout(() => { if (scenario.value === 'Loading') scenario.value = 'Populated' }, 1500)
}

function localPage(direction) {
  const tabs = destination.value === 'play' && playView.value === 'mods'
    ? ['Current set', 'Browse & add']
    : destination.value === 'settings'
    ? Object.keys(settingGroups)
    : destination.value === 'controls'
      ? ['Bindings', 'Input tuning']
      : destination.value === 'mods'
        ? ['Installed', 'Browse catalog']
        : destination.value === 'players'
          ? ['Local players', 'Profiles', 'Devices']
          : []
  const current = destination.value === 'play' && playView.value === 'mods' ? sessionModsTab : destination.value === 'settings' ? settingsTab : destination.value === 'controls' ? controlTab : destination.value === 'mods' ? modsTab : playerTab
  if (!tabs.length) return
  current.value = tabs[(tabs.indexOf(current.value) + direction + tabs.length) % tabs.length]
  nextTick(focusFirstContent)
}

function back() {
  if (modal.value) {
    capture.value = null
    closeModal()
    return
  }
  if (destination.value === 'play' && playView.value !== 'lobby') {
    playView.value = 'lobby'
    nextTick(focusFirstContent)
    return
  }
  chooseDestination('play')
}

function geometricMove(direction) {
  const active = document.activeElement
  const candidates = [...document.querySelectorAll('[data-focus]:not([disabled])')].filter((el) => el.offsetParent !== null)
  if (!candidates.length) return
  if (!candidates.includes(active)) return candidates[0].focus()
  const source = active.getBoundingClientRect()
  const sx = source.left + source.width / 2
  const sy = source.top + source.height / 2
  const vertical = direction === 'up' || direction === 'down'
  const sign = direction === 'up' || direction === 'left' ? -1 : 1
  const ranked = candidates.filter((el) => el !== active).map((el) => {
    const box = el.getBoundingClientRect()
    const dx = box.left + box.width / 2 - sx
    const dy = box.top + box.height / 2 - sy
    const primary = vertical ? dy : dx
    const secondary = vertical ? dx : dy
    return { el, primary, score: Math.abs(primary) + Math.abs(secondary) * 2.2 }
  }).filter((item) => Math.sign(item.primary) === sign).sort((a, b) => a.score - b.score)
  ranked[0]?.el.focus()
}

function onKeydown(event) {
  inputMode.value = 'controller'
  if (capture.value) return captureInput(event)
  if (event.key === 'Escape' || event.key === 'Backspace') {
    if (event.target.matches('input[type="text"]') && event.target.value) return
    event.preventDefault(); back(); return
  }
  if (event.key.toLowerCase() === 'q') { event.preventDefault(); localPage(-1); return }
  if (event.key.toLowerCase() === 'e') { event.preventDefault(); localPage(1); return }
  const map = { ArrowUp: 'up', ArrowDown: 'down', ArrowLeft: 'left', ArrowRight: 'right' }
  if (map[event.key] && !event.target.matches('input, select')) {
    event.preventDefault(); geometricMove(map[event.key])
  }
}

function onPointerMove() { inputMode.value = 'pointer' }

onMounted(() => {
  window.addEventListener('keydown', onKeydown)
  window.addEventListener('pointermove', onPointerMove, { passive: true })
  nextTick(() => document.querySelector('[data-focus]')?.focus())
})
onUnmounted(() => {
  window.removeEventListener('keydown', onKeydown)
  window.removeEventListener('pointermove', onPointerMove)
})
</script>

<template>
  <main class="game-shell" :class="`input-${inputMode}`">
    <div class="backdrop" aria-hidden="true"></div>
    <header class="topbar">
      <div class="brand">
        <span class="brand-mark">G</span>
        <div><strong>GUBSY SHELL</strong><small>Splonks design prototype</small></div>
      </div>
      <div class="top-status">
        <span class="status-dot"></span><span>{{ onlineStatus }}</span>
        <span class="divider"></span><span>{{ state.activeProfile }}</span>
      </div>
      <label class="scenario-picker">
        <span>Demo state</span>
        <select data-focus :value="scenario" @change="setScenario($event.target.value)">
          <option>Populated</option><option>Empty</option><option>Loading</option><option>Error</option>
        </select>
      </label>
    </header>

    <div class="layout">
      <aside class="rail">
        <div class="profile-card">
          <span class="avatar" :style="{ '--avatar': currentProfile?.color }">{{ state.activeProfile.slice(0, 2).toUpperCase() }}</span>
          <div><small>ACTIVE PROFILE</small><strong>{{ state.activeProfile }}</strong></div>
        </div>
        <nav aria-label="Main menu">
          <button v-for="item in navItems" :key="item.id" data-focus class="nav-item" :class="{ active: destination === item.id }" @click="chooseDestination(item.id)">
            <span class="nav-icon">{{ item.icon }}</span><span><strong>{{ item.label }}</strong><small>{{ item.hint }}</small></span><span class="chevron">›</span>
          </button>
        </nav>
        <button data-focus class="nav-item quit" @click="showModal('quit', 'Quit Splonks?', { message: 'Unsaved progress since the last checkpoint may be lost.', danger: true })">
          <span class="nav-icon">×</span><span><strong>Quit</strong><small>Return to desktop</small></span><span class="chevron">›</span>
        </button>
        <div class="rail-footer"><span>UI MOCK v0.1</span><span>144 FPS TARGET</span></div>
      </aside>

      <section class="content">
        <div class="content-heading">
          <div><p class="eyebrow">SPLONKS / {{ sectionTitle.toUpperCase() }}</p><h1>{{ sectionTitle }}</h1></div>
          <div class="heading-meta"><span class="mode-chip">{{ inputMode === 'controller' ? '⌁ Controller navigation' : '↖ Pointer navigation' }}</span></div>
        </div>

        <div v-if="scenario === 'Loading'" class="state-surface"><span class="spinner"></span><h2>Loading {{ sectionTitle.toLowerCase() }}…</h2><p>Simulating an asynchronous Gubsy service response.</p></div>
        <div v-else-if="scenario === 'Error'" class="state-surface error-state"><span class="state-icon">!</span><h2>Couldn’t load this section</h2><p>The fake provider returned a recoverable error.</p><button data-focus class="button primary" @click="scenario = 'Populated'; notify('Provider reconnected')">Retry</button></div>
        <div v-else-if="scenario === 'Empty' && destination !== 'settings'" class="state-surface"><span class="state-icon">◇</span><h2>No {{ sectionTitle.toLowerCase() }} yet</h2><p>This is the deliberate empty state for this destination.</p><button data-focus class="button primary" @click="scenario = 'Populated'">Restore sample data</button></div>

        <template v-else>
          <section v-if="destination === 'play'" class="screen play-screen">
            <div v-if="playView === 'lobby'" class="mobile-play-tabs"><button data-focus :class="{ active: mobilePlayPane === 'setup' }" @click="mobilePlayPane = 'setup'">Setup</button><button data-focus :class="{ active: mobilePlayPane === 'party' }" @click="mobilePlayPane = 'party'">Party · {{ lobbyPlayers.length }}/{{ lobbyCapacity }}</button></div>
            <div v-if="playView === 'lobby'" class="valve-lobby">
              <section class="lobby-config panel" :class="{ 'mobile-pane-hidden': mobilePlayPane !== 'setup' }">
                <div class="lobby-heading">
                  <span class="save-thumb lobby-art"></span>
                  <div><p class="eyebrow">{{ sessionMode === 'Arena' ? 'VERSUS MATCH' : sessionMode === 'New expedition' ? 'NEW QUEST' : 'CONTINUE QUEST' }}</p><h2>{{ sessionMode === 'Arena' ? 'Cavern Trials' : selectedQuest.name }}</h2><p v-if="sessionMode === 'Continue expedition'">{{ currentSave?.name }} · {{ selectedCheckpoint }} · {{ currentSave?.owner }}</p><p v-else-if="sessionMode === 'New expedition'">{{ selectedQuest.region }} · {{ selectedQuest.length }} · {{ selectedQuest.difficulty }}</p><p v-else>Competitive arenas · {{ sessionRules.teams }} · first to {{ sessionRules.rounds }}</p></div>
                  <button data-focus class="button compact" @click="playView = sessionMode === 'Arena' ? 'settings' : 'quest'">{{ sessionMode === 'Continue expedition' ? 'Checkpoint' : sessionMode === 'New expedition' ? 'Choose quest' : 'Match rules' }}</button>
                </div>

                <div class="lobby-options">
                  <label class="lobby-option"><span><strong>Activity</strong><small>Splonks supplies a different session model for each activity</small></span><select data-focus v-model="sessionMode"><option>Continue expedition</option><option>New expedition</option><option>Arena</option></select></label>
                  <button v-if="sessionMode !== 'Arena'" data-focus class="lobby-option command" @click="playView = 'quest'"><span><strong>{{ sessionMode === 'Continue expedition' ? 'Resume point' : 'Quest' }}</strong><small>{{ sessionMode === 'Continue expedition' ? `${selectedCheckpoint} · ${selectedQuest.name}` : `${selectedQuest.name} · ${selectedQuest.length}` }}</small></span><b>{{ sessionMode === 'Continue expedition' ? 'CHOOSE CHECKPOINT ›' : 'VIEW ROUTE ›' }}</b></button>
                  <label class="lobby-option"><span><strong>Play with</strong><small>Who may occupy the remaining slots</small></span><select data-focus v-model="sessionAccess"><option>Solo</option><option>Local players only</option><option>Friends can join</option><option>Invite only</option><option>Public</option></select></label>
                  <label class="lobby-option" :class="{ disabled: ['Solo','Local players only'].includes(sessionAccess) }"><span><strong>Host using</strong><small>Automatic chooses the best available route</small></span><select data-focus v-model="sessionHost" :disabled="['Solo','Local players only'].includes(sessionAccess)"><option>Automatic</option><option>Direct connection</option><option>Gubsy relay</option><option>Dedicated server</option><option>Steam lobby</option></select></label>
                  <button data-focus class="lobby-option command" @click="playView = 'settings'"><span><strong>{{ sessionMode === 'Arena' ? 'Match rules' : 'Expedition rules' }}</strong><small v-if="sessionMode === 'Arena'">{{ sessionRules.teams }} · {{ sessionRules.damage }}% damage · {{ sessionRules.speed }}% speed</small><small v-else>{{ sessionRules.difficulty }} · {{ sessionRules.lives }} lives · ghost at {{ sessionRules.ghostTimer }}s</small></span><b>EDIT ALL ›</b></button>
                  <button data-focus class="lobby-option command" @click="playView = 'mods'; sessionModsTab = 'Current set'"><span><strong>Session mods</strong><small>{{ sessionContentSummary }}</small></span><b>MANAGE ›</b></button>
                </div>

                <div class="lobby-bottom">
                  <button data-focus class="button" @click="showModal('pause', sessionRunning ? 'Game paused' : 'In-game menu preview',{ message: sessionRunning ? 'The running session is suspended while this menu is open.' : 'Start the session first, or preview the same shell in an in-game context.' })">{{ sessionRunning ? 'Open in-game menu' : 'Pause preview' }}</button>
                  <button data-focus class="button primary large" @click="startSession">▶ {{ sessionMode === 'Continue expedition' ? `Resume ${selectedCheckpoint}` : sessionMode === 'Arena' ? 'Start match' : 'Begin expedition' }}</button>
                </div>
              </section>

              <aside class="party-panel panel" :class="{ 'mobile-pane-hidden': mobilePlayPane !== 'party' }">
                <div class="panel-title"><div><p class="eyebrow">PLAYERS</p><h3>Your party</h3></div><span class="party-state">{{ onlineStatus }}</span></div>
                <div class="party-slots">
                  <button v-for="(player,index) in lobbyPlayers" :key="player.id" data-focus class="party-slot" @click="selectedPlayer = state.players.findIndex(item => item.id === player.id); chooseDestination('players')"><span class="player-number">P{{ index + 1 }}</span><span><strong>{{ player.name }}</strong><small>{{ player.device }}</small></span><b>{{ player.ready ? 'READY' : 'JOIN' }}</b></button>
                  <button v-for="slot in Math.max(0, lobbyCapacity - lobbyPlayers.length)" :key="`empty-${slot}`" data-focus class="party-slot empty" @click="sessionAccess === 'Local players only' ? addLocalPlayer() : copyInviteLink()"><span class="player-number">＋</span><span><strong>Open slot</strong><small>{{ sessionAccess === 'Local players only' ? 'Press a local controller button' : 'Invite a friend or add locally' }}</small></span></button>
                </div>
                <div v-if="sessionAccess !== 'Solo'" class="party-actions">
                  <button data-focus class="button" :disabled="['Solo','Local players only'].includes(sessionAccess)" @click="copyInviteLink">Invite / copy link</button>
                  <button data-focus class="button" @click="showModal('find-game','Join another game')">Friends & public games</button>
                </div>
                <div v-if="sessionAccess === 'Solo'" class="solo-note"><strong>Solo expedition</strong><small>No empty network or local slots exist in this session. Change “Play with” to open the party.</small></div>
                <div class="session-summary"><span>CONTENT</span><strong>{{ state.mods.filter(mod => mod.sessionEnabled).length }} mods</strong><span>RULESET</span><strong>{{ sessionMode === 'Arena' ? sessionRules.teams : sessionRules.difficulty }}</strong><span>NETWORK</span><strong>{{ ['Solo','Local players only'].includes(sessionAccess) ? 'None' : sessionHost }}</strong></div>
              </aside>
            </div>
            <div v-else-if="playView === 'mods'" class="session-mod-workspace">
              <header class="session-mod-header panel" :class="{ runtime: sessionRunning }">
                <button data-focus class="button" @click="playView = 'lobby'">‹ Back to lobby</button>
                <div><p class="eyebrow">{{ sessionRunning ? 'RUNNING SESSION / CONTENT' : 'CURRENT LOBBY / CONTENT' }}</p><h2>Session mods <span v-if="sessionRunning" class="runtime-badge">LIVE</span></h2><p v-if="sessionRunning">Mods declare whether changes are immediate, queued for a safe boundary, or intended for a new session.</p><p v-else-if="sessionMode === 'Continue expedition'">Checkpoint “{{ selectedCheckpoint }}” expects {{ savedModStatus.manifest.length }} exact {{ savedModStatus.manifest.length === 1 ? 'mod' : 'mods' }}.</p><p v-else-if="sessionMode === 'New expedition'">Choose the content for this new expedition. Its exact versions will be recorded when play begins.</p><p v-else>Choose the content required by this match. Joining players must resolve the same set.</p></div>
                <div class="session-mod-tabs"><button v-for="tab in ['Current set','Browse & add']" :key="tab" data-focus :class="{ active: sessionModsTab === tab }" @click="sessionModsTab = tab">{{ tab }}</button></div>
              </header>
              <div v-if="sessionModsTab === 'Current set'" class="manifest-banner" :class="{ exact: sessionMode !== 'Continue expedition' || savedModStatus.exact }">
                <div v-if="sessionMode === 'Continue expedition'"><strong>{{ savedModStatus.exact ? '✓ Lobby matches checkpoint manifest' : 'Checkpoint mod set differs from this lobby' }}</strong><small v-if="!savedModStatus.exact">{{ savedModStatus.missing.length }} missing · {{ savedModStatus.inactive.length }} inactive · {{ savedModStatus.extras.length }} extra</small><small v-else>Every recorded package and version is active; no extra content.</small></div>
                <div v-else-if="sessionMode === 'New expedition'"><strong>New expedition manifest</strong><small>{{ activeSessionMods.length }} active packages will be recorded with the new expedition when it starts.</small></div>
                <div v-else><strong>Match content requirement</strong><small>{{ activeSessionMods.length }} active packages will be synchronized with every player before launch.</small></div>
                <button v-if="sessionMode === 'Continue expedition' && !savedModStatus.exact" data-focus class="button primary" @click="syncModsToProgress">Install & sync checkpoint set</button>
              </div>
              <div v-if="sessionModsTab === 'Browse & add'" class="session-catalog-toolbar"><label class="search"><span>⌕</span><input data-focus v-model="sessionModSearch" placeholder="Find a mod to add to this lobby…"></label><span>Install and activation happen together here.</span></div>
              <div class="session-mod-layout">
                <div v-if="sessionModsTab === 'Current set'" class="mod-list panel" :class="{ 'mobile-pane-hidden': mobileModDetail }">
                  <div class="catalog-count">{{ state.mods.filter(mod => mod.sessionEnabled).length }} ACTIVE / {{ state.mods.length }} INSTALLED</div>
                  <button v-for="mod in state.mods" :key="mod.id" data-focus class="session-set-row" :class="{ selected: currentSessionMod?.id === mod.id }" @click="sessionModSelection = mod.id; mobileModDetail = true">
                    <span class="mod-thumb" :style="modThumbStyle(mod)"></span><span><strong>{{ mod.name }}</strong><small>v{{ mod.version }} · {{ sessionRunning ? runtimePolicyLabel(mod) : `${mod.dependencies?.length || 0} required dependencies` }}</small><em v-if="mod.pendingRuntime">QUEUED: {{ mod.pendingRuntime }}</em></span>
                    <span class="session-toggle" :class="{ on: mod.sessionEnabled, locked: mod.required, update: mod.status === 'Needs update' }" @click.stop="toggleSessionMod(mod)">{{ mod.required ? 'LOCKED' : mod.status === 'Needs update' ? `UPDATE → ${mod.latestVersion}` : mod.sessionEnabled ? 'ACTIVE' : 'OFF' }}</span>
                  </button>
                </div>
                <div v-else class="mod-list panel" :class="{ 'mobile-pane-hidden': mobileModDetail }">
                  <div class="catalog-count">{{ filteredSessionCatalog.length }} MODS · SELECT TO INSPECT</div>
                  <button v-for="mod in filteredSessionCatalog" :key="mod.id" data-focus class="mod-row catalog-row" :class="{ selected: currentSessionCatalogMod?.id === mod.id, incompatible: mod.installable === false }" @click="sessionCatalogSelection = mod.id; mobileModDetail = true"><span class="mod-thumb" :style="modThumbStyle(mod)"></span><span><strong>{{ mod.name }}</strong><small>{{ mod.category }} · {{ mod.downloads }} downloads</small><em v-if="mod.dependencies?.length">Requires {{ mod.dependencies.join(', ') }}</em></span><b>{{ mod.installable === false ? 'UNAVAILABLE' : findMod(mod.name)?.sessionEnabled ? 'ACTIVE' : findMod(mod.name) ? 'INSTALLED' : 'ADD' }}</b></button>
                </div>
                <aside v-if="sessionModsTab === 'Current set' && currentSessionMod" class="detail-panel panel session-mod-detail" :class="{ 'mobile-pane-hidden': !mobileModDetail }">
                  <button data-focus class="mobile-pane-back" @click="mobileModDetail = false">‹ All session mods</button>
                  <span class="mod-detail-art" :style="modThumbStyle(currentSessionMod)"></span><p class="eyebrow">SESSION CONTENT</p><h2>{{ currentSessionMod.name }}</h2><p>{{ currentSessionMod.description }}</p>
                  <div v-if="currentSessionMod.status === 'Needs update'" class="compatibility-banner bad"><span>↑</span><div><strong>Update required before use</strong><small>Installed v{{ currentSessionMod.version }} → compatible v{{ currentSessionMod.latestVersion }}</small></div><button data-focus class="button primary" @click="updateMod(currentSessionMod, true)">Update & enable</button></div>
                  <div v-if="sessionRunning" class="runtime-policy" :class="runtimePolicy(currentSessionMod)"><p class="eyebrow">WHILE PLAYING</p><strong>{{ runtimePolicyLabel(currentSessionMod) }}</strong><small v-if="runtimePolicy(currentSessionMod) === 'hot-safe'">The mod declares reversible runtime hooks and can migrate or preserve its state immediately.</small><small v-else-if="runtimePolicy(currentSessionMod) === 'next-stage'">Queueing avoids missing content generation that already occurred in the current stage.</small><small v-else>Existing world generation cannot be reconstructed safely. The current world may retain orphaned content.</small></div>
                  <div class="relationship-block"><strong>Required dependencies</strong><div v-if="!dependencyRows(currentSessionMod).filter(row => row.kind === 'Required').length" class="relationship-empty">None — this is a root package.</div><div v-for="row in dependencyRows(currentSessionMod).filter(row => row.kind === 'Required')" :key="row.name" class="relationship-row"><span>↳ {{ row.name }}</span><b :class="{ warning: !row.installed }">{{ row.installed ? 'INSTALLED' : 'MISSING' }}</b></div></div>
                  <div class="relationship-block"><strong>Required by installed mods</strong><div v-if="!dependentMods(currentSessionMod.name).length" class="relationship-empty">No installed mod depends on this.</div><div v-for="mod in dependentMods(currentSessionMod.name)" :key="mod.id" class="relationship-row"><span>↑ {{ mod.name }}</span><b>{{ mod.sessionEnabled ? 'ACTIVE' : 'INSTALLED' }}</b></div></div>
                  <div class="manifest-membership"><strong>{{ sessionMode === 'Continue expedition' ? 'Checkpoint manifest' : sessionMode === 'New expedition' ? 'New expedition manifest' : 'Match requirement' }}</strong><template v-if="sessionMode === 'Continue expedition'"><span v-if="savedModStatus.manifest.some(entry => entry.name === currentSessionMod.name)">Requires v{{ savedModStatus.manifest.find(entry => entry.name === currentSessionMod.name)?.version }}</span><span v-else>Not used by this checkpoint</span></template><span v-else-if="currentSessionMod.sessionEnabled">{{ sessionMode === 'New expedition' ? 'Will record' : 'Required at' }} v{{ currentSessionMod.version }}</span><span v-else>Not included</span></div>
                </aside>
                <aside v-else-if="currentSessionCatalogMod" class="detail-panel panel session-mod-detail" :class="{ 'mobile-pane-hidden': !mobileModDetail }">
                  <button data-focus class="mobile-pane-back" @click="mobileModDetail = false">‹ Browse results</button>
                  <span class="mod-detail-art" :style="modThumbStyle(currentSessionCatalogMod)"></span><p class="eyebrow">ADD TO THIS LOBBY</p><h2>{{ currentSessionCatalogMod.name }}</h2><p>{{ currentSessionCatalogMod.description }}</p>
                  <div class="compatibility-banner" :class="{ bad: currentSessionCatalogMod.installable === false }"><span>{{ currentSessionCatalogMod.installable === false ? '×' : '✓' }}</span><div><strong>{{ currentSessionCatalogMod.installable === false ? 'Unavailable' : 'Compatible' }}</strong><small>{{ modCompatibility(currentSessionCatalogMod) }}</small></div></div>
                  <div class="relationship-block"><strong>Dependency plan</strong><div v-if="!dependencyRows(currentSessionCatalogMod).length" class="relationship-empty">No additional packages required.</div><div v-for="row in dependencyRows(currentSessionCatalogMod)" :key="`${row.kind}-${row.name}`" class="relationship-row"><span>{{ row.kind === 'Required' ? '↳' : '◇' }} {{ row.name }}</span><b>{{ row.kind }} · {{ row.installed ? 'installed' : 'auto-install' }}</b></div></div>
                  <button data-focus class="button primary" :disabled="currentSessionCatalogMod.installable === false || findMod(currentSessionCatalogMod.name)?.sessionEnabled" @click="installCatalogMod(currentSessionCatalogMod, true)">{{ findMod(currentSessionCatalogMod.name)?.sessionEnabled ? 'Active in lobby' : findMod(currentSessionCatalogMod.name) ? 'Add installed mod' : `Install${currentSessionCatalogMod.dependencies?.length ? ' dependencies' : ''} & add` }}</button>
                  <p class="capture-note">One action downloads the mod and all required dependencies, then activates the resolved set for this lobby.</p>
                </aside>
              </div>
            </div>
            <div v-else-if="playView === 'quest'" class="play-subview">
              <header class="play-subview-header panel"><button data-focus class="button" @click="playView = 'lobby'">‹ Back to lobby</button><div><p class="eyebrow">SPLONKS QUEST PROVIDER</p><h2>{{ sessionMode === 'New expedition' ? 'Choose a quest' : 'Choose a resume point' }}</h2><p>{{ sessionMode === 'New expedition' ? 'A new expedition begins at stage one; no arbitrary name is required.' : 'Continue data selects a quest, its owning profile, and a checkpoint inside that quest.' }}</p></div></header>
              <div class="quest-workspace">
                <div class="quest-browser panel" :class="{ 'mobile-pane-hidden': mobileQuestDetail }">
                  <template v-if="sessionMode === 'New expedition'">
                    <p class="eyebrow">AVAILABLE QUESTS</p>
                    <button v-for="quest in quests" :key="quest.id" data-focus class="quest-row" :class="{ selected: selectedQuest.id === quest.id }" @click="selectedQuestId = quest.id; mobileQuestDetail = true"><span>{{ quest.stages.length }}</span><div><strong>{{ quest.name }}</strong><small>{{ quest.region }} · {{ quest.difficulty }}</small></div><b>QUEST</b></button>
                  </template>
                  <template v-else>
                    <p class="eyebrow">EXPEDITIONS</p>
                    <button v-for="save in state.saves.filter(item => item.status === 'ready')" :key="save.id" data-focus class="quest-row compact" :class="{ selected: selectedSave === save.id }" @click="selectedSave = save.id"><span>◈</span><div><strong>{{ save.name }}</strong><small>{{ save.owner }} · {{ save.time }}</small></div></button>
                    <p class="eyebrow checkpoint-label">CHECKPOINTS IN {{ currentSave.name.toUpperCase() }}</p>
                    <button v-for="checkpoint in currentSave.checkpoints" :key="checkpoint.name" data-focus class="checkpoint-choice" :class="{ selected: selectedCheckpoint === checkpoint.name }" @click="selectedCheckpoint = checkpoint.name; mobileQuestDetail = true"><span></span><div><strong>{{ checkpoint.name }}</strong><small>{{ checkpoint.stamp }}</small></div><b v-if="checkpoint.current">LATEST</b></button>
                  </template>
                </div>
                <article class="quest-detail panel" :class="{ 'mobile-pane-hidden': !mobileQuestDetail }">
                  <button data-focus class="mobile-pane-back" @click="mobileQuestDetail = false">‹ Quest choices</button>
                  <div class="quest-hero"><div><p class="eyebrow">{{ selectedQuest.region }}</p><h2>{{ selectedQuest.name }}</h2><p>{{ selectedQuest.description }}</p><div class="quest-meta"><span>{{ selectedQuest.length }}</span><span>{{ selectedQuest.difficulty }}</span><span>{{ sessionMode === 'Continue expedition' ? currentSave.time : 'Fresh route' }}</span></div></div></div>
                  <div class="quest-route"><p class="eyebrow">QUEST ROUTE</p><div class="route-line"><div v-for="(stage,index) in selectedQuest.stages" :key="stage" class="route-stage" :class="{ reached: sessionMode === 'Continue expedition' && index < Math.min(currentSave.checkpoints.length + 1, selectedQuest.stages.length), selected: stage.toLowerCase().includes((selectedCheckpoint || '').split(' ')[0].toLowerCase()) }"><span>{{ index + 1 }}</span><strong>{{ stage }}</strong></div></div></div>
                  <div class="quest-context"><strong>{{ sessionMode === 'New expedition' ? 'New expedition data' : 'Checkpoint payload' }}</strong><p v-if="sessionMode === 'New expedition'">Starts {{ selectedQuest.name }} at {{ selectedQuest.stages[0] }} with {{ state.activeProfile }}, the current lobby rules, players, and active mod manifest.</p><p v-else>Resumes {{ currentSave.name }} at {{ selectedCheckpoint }} as {{ currentSave.owner }}. Inventory, quest flags, world seed, and exact mods come from this checkpoint.</p></div>
                  <div class="action-row"><button data-focus class="button primary" @click="playView = 'lobby'; notify(sessionMode === 'New expedition' ? `${selectedQuest.name} selected` : `${selectedCheckpoint} selected`)">{{ sessionMode === 'New expedition' ? 'Use this quest' : 'Resume from this checkpoint' }}</button></div>
                </article>
              </div>
            </div>
            <div v-else-if="playView === 'settings'" class="play-subview rules-workspace">
              <header class="play-subview-header panel"><button data-focus class="button" @click="playView = 'lobby'">‹ Back to lobby</button><div><p class="eyebrow">SPLONKS SESSION RULES</p><h2>{{ sessionMode === 'Arena' ? 'Cavern Trials match settings' : 'Expedition settings' }}</h2><p>{{ activeRuleDefinitions.length }} settings · {{ modRuleDefinitions.length }} contributed by active mods.</p></div><button data-focus class="button" @click="resetSessionRules">Reset mode defaults</button></header>
              <div class="rules-layout">
                <div class="rules-list panel" :class="{ 'mobile-pane-hidden': mobileRuleDetail }">
                  <div v-for="rule in activeRuleDefinitions" :key="rule.key" data-focus tabindex="0" class="rule-row" :class="{ selected: currentRuleDefinition.key === rule.key, contributed: rule.source }" @click="selectedRuleKey = rule.key; mobileRuleDetail = true"><div><strong>{{ rule.label }}</strong><small>{{ rule.note }}</small><em v-if="rule.source">MOD · {{ rule.source }}</em></div><button v-if="rule.type === 'toggle'" data-focus class="toggle" :class="{ on: sessionRules[rule.key] }" @click.stop="sessionRules[rule.key] = !sessionRules[rule.key]"><span></span>{{ sessionRules[rule.key] ? 'ON' : 'OFF' }}</button><select v-else-if="rule.type === 'select'" data-focus v-model="sessionRules[rule.key]" @click.stop><option v-for="option in rule.options" :key="option">{{ option }}</option></select><label v-else class="range-control" @click.stop><input data-focus type="range" :min="rule.min" :max="rule.max" :step="rule.step || 1" v-model.number="sessionRules[rule.key]"><output>{{ sessionRules[rule.key] }}{{ rule.unit }}</output></label></div>
                </div>
                <aside class="rule-detail panel" :class="{ 'mobile-pane-hidden': !mobileRuleDetail }">
                  <button data-focus class="mobile-pane-back" @click="mobileRuleDetail = false">‹ All rules</button>
                  <p class="eyebrow">SELECTED RULE</p><h2>{{ currentRuleDefinition.label }}</h2><span class="rule-source" :class="{ mod: currentRuleDefinition.source }">{{ currentRuleDefinition.source ? `Provided by mod · ${currentRuleDefinition.source}` : 'Provided by Splonks' }}</span><p>{{ currentRuleDefinition.detail }}</p><div class="rule-value"><span>CURRENT VALUE</span><strong>{{ typeof sessionRules[currentRuleDefinition.key] === 'boolean' ? (sessionRules[currentRuleDefinition.key] ? 'Enabled' : 'Disabled') : `${sessionRules[currentRuleDefinition.key]}${currentRuleDefinition.unit || ''}` }}</strong></div>
                  <div class="rule-impact"><p class="eyebrow">SESSION EFFECT</p><div><span>Activity</span><strong>{{ sessionMode }}</strong></div><div><span>Applies to</span><strong>{{ sessionMode === 'Arena' ? 'Every round' : sessionMode === 'Continue expedition' ? 'Future stages' : 'Entire new quest' }}</strong></div><div><span>Authority</span><strong>{{ sessionAccess === 'Solo' ? 'Local player' : 'Lobby host' }}</strong></div><div><span>Synced</span><strong>{{ sessionAccess === 'Solo' ? 'Not required' : 'Before launch' }}</strong></div></div>
                  <p class="capture-note">The game supplies these rule definitions and descriptions. Gubsy supplies the stable editor widgets, focus behavior, serialization, and host synchronization.</p>
                </aside>
              </div>
            </div>
          </section>

          <section v-else-if="destination === 'players'" class="screen">
            <div class="subnav"><button v-for="tab in ['Local players','Profiles','Devices']" :key="tab" data-focus :class="{ active: playerTab === tab }" @click="playerTab = tab">{{ tab }}</button><span class="bumper-hint"><kbd>Q</kbd><kbd>E</kbd> change section</span></div>
            <template v-if="playerTab === 'Local players'">
              <div class="split-view">
                <div class="list-panel panel"><div class="panel-title"><div><p class="eyebrow">LOCAL ROSTER</p><h3>{{ state.players.length }} / 4 players</h3></div><button data-focus class="icon-button" :disabled="onlineStatus !== 'Offline'" title="Roster is locked during network sessions" @click="addLocalPlayer">＋</button></div>
                  <button v-for="(player, index) in state.players" :key="player.id" data-focus class="list-row" :class="{ selected: selectedPlayer === index }" @click="selectedPlayer = index"><span class="player-number">P{{ index + 1 }}</span><span><strong>{{ player.name }}</strong><small>{{ player.device }}</small></span><span class="ready" :class="{ off: !player.ready }">{{ player.ready ? 'READY' : 'OPEN' }}</span></button>
                </div>
                <div class="detail-panel panel" v-if="currentPlayer"><p class="eyebrow">PLAYER {{ selectedPlayer + 1 }} DETAILS</p><h2>{{ currentPlayer.name }}</h2>
                  <label class="field"><span>Player profile</span><select data-focus v-model="currentPlayer.profile"><option v-for="p in state.profiles" :key="p.id">{{ p.name }}</option></select></label>
                  <label class="field"><span>Bindings</span><select data-focus v-model="currentPlayer.binds"><option v-for="p in state.bindProfiles" :key="p.id">{{ p.name }}</option></select></label>
                  <label class="field"><span>Input tuning</span><select data-focus v-model="currentPlayer.input"><option>Standard</option><option>Low deadzone</option><option>Southpaw</option></select></label>
                  <label class="field"><span>Assigned device</span><select data-focus v-model="currentPlayer.device"><option>Press A to join</option><option v-for="device in state.devices" :key="device">{{ device }}</option></select></label>
                  <div class="action-row"><button data-focus class="button" @click="currentPlayer.ready = !currentPlayer.ready">{{ currentPlayer.ready ? 'Mark not ready' : 'Mark ready' }}</button><button data-focus class="button danger" :disabled="onlineStatus !== 'Offline'" @click="removeLocalPlayer">Remove</button></div><p v-if="onlineStatus !== 'Offline'" class="warning">The local roster is locked while a network session is active.</p>
                </div>
              </div>
            </template>
            <template v-else-if="playerTab === 'Profiles'">
              <div class="toolbar"><span>{{ state.profiles.length }} profiles</span><button data-focus class="button primary" @click="showModal('new-profile','Create player profile',{ value: 'New Player' })">＋ New profile</button></div>
              <div class="card-grid three"><article v-for="profile in state.profiles" :key="profile.id" class="profile-tile" :class="{ selected: state.activeProfile === profile.name }"><button data-focus class="tile-main" @click="state.activeProfile = profile.name; notify(`${profile.name} is now active`)"><span class="avatar large-avatar" :style="{ '--avatar': profile.color }">{{ profile.name.slice(0,2).toUpperCase() }}</span><strong>{{ profile.name }}</strong><small>{{ profile.played }} · {{ profile.stats.runs }} runs · {{ profile.stats.wins }} wins</small><span class="selected-label">{{ state.activeProfile === profile.name ? 'ACTIVE' : 'SELECT' }}</span></button><div class="tile-actions profile-actions"><button data-focus @click="showModal('profile-history','Player history',{ payload: profile.id })">History</button><button data-focus @click="showModal('rename-profile','Rename profile',{ value: profile.name, payload: profile.id })">Rename</button><button data-focus @click="showModal('reset-profile','Reset profile preferences?',{ payload: profile.id, message: 'Settings and assignments return to defaults. Save progress is preserved.', danger: true })">Reset</button><button data-focus :disabled="state.profiles.length <= 1" @click="showModal('delete-profile','Delete profile?',{ payload: profile.id, message: `Delete ${profile.name}? Save data will not be removed.`, danger: true })">Delete</button></div></article></div>
            </template>
            <template v-else>
              <div class="toolbar"><span>{{ state.devices.length }} detected devices</span><button data-focus class="button" @click="refreshDevices">↻ Refresh devices</button></div>
              <div class="stack"><article v-for="(device,index) in state.devices" :key="device" class="device-row panel"><span class="device-icon">{{ index === 1 ? '⌨' : '⌁' }}</span><div><strong>{{ device }}</strong><small>{{ state.players.find(p => p.device === device)?.name || 'Not assigned' }}</small></div><span class="connection">CONNECTED</span><button data-focus class="button compact" @click="notify(`${device} identification pulse sent`)">Identify</button></article></div>
            </template>
          </section>

          <section v-else-if="destination === 'online'" class="screen">
            <div class="subnav"><button v-for="tab in ['Lobby','Host','Join','Browse']" :key="tab" data-focus :class="{ active: onlineTab === tab }" @click="onlineTab = tab">{{ tab }}</button><span class="bumper-hint"><kbd>Q</kbd><kbd>E</kbd> change section</span></div>
            <div v-if="onlineTab === 'Lobby'" class="split-view lobby-view"><div class="panel"><p class="eyebrow">SESSION STATUS</p><h2>{{ onlineStatus }}</h2><label class="field state-debug"><span>Preview network state</span><select data-focus v-model="onlineStatus"><option>Offline</option><option>Connecting</option><option>Hosting · Ready</option><option>Client · Waiting for host</option><option>Session full</option><option>Game in progress</option><option>Disconnected · Retry available</option></select></label><div class="lobby-code" v-if="onlineStatus.includes('Hosting')"><span>ROOM CODE</span><strong>VIOLET-7</strong></div><div class="stack compact-stack"><div v-for="(player,index) in state.players" :key="player.id" class="roster-row"><span>P{{ index+1 }}</span><strong>{{ player.name }}</strong><small>{{ player.device }}</small><b :class="{ off: !player.ready }">{{ player.ready ? 'READY' : 'WAITING' }}</b></div><div v-if="onlineStatus !== 'Offline' && !onlineStatus.includes('Local')" class="roster-row remote"><span>R1</span><strong>Fern</strong><small>Remote player · edits host-controlled</small><b>READY</b></div></div></div><div class="panel"><p class="eyebrow">SESSION SETUP</p><h3>Glass Caverns Run</h3><label class="field"><span>Max players</span><select data-focus><option>2 players</option><option selected>4 players</option></select></label><label class="field"><span>Join permission</span><select data-focus><option>Friends</option><option>Anyone</option><option>Invite only</option></select></label><label class="toggle-row"><span><strong>Friendly fire</strong><small>Players can damage each other</small></span><input data-focus type="checkbox"></label><div class="action-row bottom"><button data-focus class="button primary" :disabled="onlineStatus.includes('Client') || onlineStatus.includes('full') || onlineStatus.includes('progress')" @click="notify('Start game command sent')">Start game</button><button v-if="onlineStatus.includes('Disconnected')" data-focus class="button primary" @click="startHosting('join')">Retry</button><button data-focus class="button" @click="onlineStatus = 'Offline'">Leave lobby</button></div></div></div>
            <div v-else-if="onlineTab === 'Host'" class="option-grid"><button data-focus class="option-card" @click="startHosting('local')"><span>⌂</span><h3>Local game</h3><p>Play together on this machine. No network session.</p><b>START LOCAL</b></button><button data-focus class="option-card" @click="startHosting('direct')"><span>↗</span><h3>Host direct</h3><p>Host through your public address and configured port.</p><b>HOST DIRECT</b></button><button data-focus class="option-card" @click="startHosting('room')"><span>◎</span><h3>Host room</h3><p>Create a discoverable room code through the lobby service.</p><b>CREATE ROOM</b></button></div>
            <div v-else-if="onlineTab === 'Join'" class="split-view"><div class="panel"><p class="eyebrow">JOIN BY ADDRESS</p><h3>Direct connection</h3><label class="field"><span>Host address</span><input data-focus value="192.168.1.42"></label><label class="field"><span>Port</span><input data-focus value="24561"></label><button data-focus class="button primary" @click="startHosting('join')">Connect</button></div><div class="panel"><p class="eyebrow">JOIN BY CODE</p><h3>Room code</h3><label class="field"><span>Six or eight character code</span><input data-focus value="VIOLET-7"></label><button data-focus class="button primary" @click="startHosting('join')">Find room</button></div></div>
            <div v-else class="panel browser"><div class="toolbar"><div><p class="eyebrow">PUBLIC ROOMS</p><h3>{{ state.rooms.length }} rooms found</h3></div><button data-focus class="button" @click="notify('Room list refreshed')">↻ Refresh</button></div><button v-for="room in state.rooms" :key="room.id" data-focus class="server-row" :class="{ selected: selectedRoom === room.id, disabled: room.status === 'Full' }" @click="selectedRoom = room.id"><span><strong>{{ room.name }}</strong><small>Hosted by {{ room.host }} · {{ room.region }}</small></span><span>{{ room.players }}</span><span>{{ room.ping }} ms</span><b>{{ room.status }}</b></button><div class="action-row"><button data-focus class="button primary" :disabled="!currentRoom || currentRoom.status === 'Full'" @click="joinRoom(currentRoom)">Join selected room</button><span v-if="currentRoom">{{ currentRoom.name }} · {{ currentRoom.region }}</span></div></div>
          </section>

          <section v-else-if="destination === 'settings'" class="screen">
            <div class="subnav"><button v-for="tab in Object.keys(settingGroups)" :key="tab" data-focus :class="{ active: settingsTab === tab }" @click="settingsTab = tab">{{ tab }}</button><span class="bumper-hint"><kbd>Q</kbd><kbd>E</kbd> change category</span></div>
            <div class="settings-layout"><div class="settings-list panel"><div class="panel-title"><div><p class="eyebrow">{{ settingsTab.toUpperCase() }}</p><h3>{{ settingGroups[settingsTab].length }} options</h3></div><span class="save-indicator">● SAVED LOCALLY</span></div>
              <div v-for="setting in settingGroups[settingsTab]" :key="setting.key" class="setting-row"><div><strong>{{ setting.label }}</strong><small>{{ setting.note }}</small></div><button v-if="setting.type === 'toggle'" data-focus class="toggle" :class="{ on: state.settings[setting.key] }" @click="state.settings[setting.key] = !state.settings[setting.key]"><span></span>{{ state.settings[setting.key] ? 'ON' : 'OFF' }}</button><select v-else-if="setting.type === 'select'" data-focus v-model="state.settings[setting.key]"><option v-for="option in setting.options" :key="option">{{ option }}</option></select><label v-else class="range-control"><input data-focus type="range" :min="setting.min ?? 0" :max="setting.max ?? 100" v-model.number="state.settings[setting.key]"><output>{{ state.settings[setting.key] }}%</output></label></div>
            </div><aside class="preview-panel panel"><p class="eyebrow">LIVE PREVIEW</p><div class="text-preview" :style="{ fontSize: `${0.82 + ({Small:0,Standard:.08,Large:.18,'Extra large':.3}[state.settings.textScale] || 0)}rem` }"><strong>Readable at final output size</strong><p>The menu is rasterized for the actual drawable resolution, not an undersized intermediate target.</p></div><div class="brightness-preview" :style="{ filter: `brightness(${state.settings.brightness / 64})` }"></div><div class="stack"><button v-if="settingsTab === 'Display'" data-focus class="button primary" @click="showModal('apply-display','Keep these display settings?',{ message: 'The native shell will revert automatically unless confirmed within 15 seconds.' })">Apply display mode</button><button data-focus class="button" @click="showModal('reset-demo','Restore every demo setting?',{ message: 'Profiles, bindings, saves, rooms, and settings will return to their sample values.', danger: true })">Reset all demo data</button></div></aside></div>
          </section>

          <section v-else-if="destination === 'controls'" class="screen controls-screen">
            <div class="subnav"><button v-for="tab in ['Bindings','Devices','Input tuning']" :key="tab" data-focus :class="{ active: controlTab === tab }" @click="controlTab = tab">{{ tab }}</button><span class="bumper-hint"><kbd>Q</kbd><kbd>E</kbd> change section</span></div>

            <template v-if="controlTab === 'Bindings'">
              <div class="toolbar controls-toolbar">
                <label class="search"><span>⌕</span><input data-focus v-model="search" placeholder="Filter actions…"></label>
                <select data-focus v-model="state.bindProfile"><option v-for="profile in state.bindProfiles" :key="profile.id">{{ profile.name }}</option></select>
                <div class="action-row bind-profile-actions"><button data-focus class="button compact" @click="showModal('new-bind-profile','New binding profile',{ value: 'Custom Binds' })">＋ New</button><button data-focus class="button compact" @click="showModal('rename-bind-profile','Rename binding profile',{ value: currentBindProfile?.name, payload: currentBindProfile?.id })">Rename</button><button data-focus class="button compact" @click="showModal('reset-bind-profile','Reset all bindings?',{ message: 'Registered actions return to their default keyboard and controller bindings.', danger: true })">Reset</button><button data-focus class="button compact danger" :disabled="state.bindProfiles.length <= 1" @click="showModal('delete-bind-profile','Delete binding profile?',{ payload: currentBindProfile?.id, message: 'Players using it will fall back to the first available profile.', danger: true })">Delete</button></div>
              </div>
              <div class="bindings-layout">
                <div class="binding-list panel"><button v-for="([action, binds]) in filteredBindings" :key="action" data-focus class="binding-row" :class="{ selected: selectedAction === action }" @click="selectedAction = action"><span><strong>{{ action }}</strong><small>{{ bindingActionMeta[action]?.kind || 'Digital action' }} · {{ bindingActionMeta[action]?.category || 'Game' }}</small></span><span class="bind-summary">{{ binds[0] || 'UNBOUND' }}</span></button></div>
                <div class="detail-panel panel binding-detail"><p class="eyebrow">SELECTED LOGICAL OUTPUT</p><h2>{{ selectedAction }}</h2><p class="muted">{{ currentBindingMeta.kind }}. Multiple physical controls and conversion rules may feed this output.</p><div v-if="currentBindingMeta.category.includes('Run group')" class="binding-group-note"><span>COMPOSED CONTROL</span><strong>Digital Run and analog Run Trigger are merged by Splonks at gameplay sampling.</strong></div><div class="bind-slots"><div v-for="(binding,index) in state.bindings[selectedAction]" :key="`${binding}-${index}`" class="bind-slot"><span>{{ index+1 }}</span><strong>{{ binding }}</strong><button data-focus @click="beginCapture(selectedAction,index)">Listen</button><button data-focus @click="beginManualBinding(selectedAction,index)">Choose</button><button data-focus class="remove" @click="removeBinding(selectedAction,index)">×</button></div></div><div class="action-row binding-add-actions"><button data-focus class="button primary" @click="beginCapture(selectedAction)">◎ Listen for input</button><button data-focus class="button" @click="beginManualBinding(selectedAction)">⌕ Choose manually</button></div><p class="capture-note">Listening is the fast path. Manual selection exposes every recognized device input and conversions such as axis → button threshold or button → full analog value.</p></div>
              </div>
            </template>

            <template v-else-if="controlTab === 'Devices'">
              <div class="device-summary-bar"><span><strong>{{ state.inputDevices.length }} recognized devices</strong><small>A local player may own any number of keyboards, pads, wheels, pedals, sticks, or HID controls.</small></span><button data-focus class="button compact" @click="refreshDevices">↻ Rescan</button></div>
              <div class="bindings-layout device-workspace">
                <div class="binding-list panel"><button v-for="device in state.inputDevices" :key="device.id" data-focus class="binding-row device-choice" :class="{ selected: selectedInputDeviceId === device.id }" @click="selectedInputDeviceId = device.id"><span><strong>{{ device.name }}</strong><small>{{ device.type }} · {{ device.connection }}</small></span><span class="device-owner-summary">{{ device.assignedPlayerIds.length ? device.assignedPlayerIds.map(id => state.players.find(player => player.id === id)?.name || `P${id}`).join(', ') : 'UNASSIGNED' }}</span></button></div>
                <div v-if="currentInputDevice" class="detail-panel panel device-detail"><p class="eyebrow">RECOGNIZED INPUT DEVICE</p><h2>{{ currentInputDevice.name }}</h2><p class="muted">{{ currentInputDevice.connection }}. Assign this device to one local player; each player may own as many separate devices as needed.</p><div class="device-assignments"><span class="section-label">DEVICE OWNER</span><button data-focus class="device-assignment" :class="{ selected: !currentInputDevice.assignedPlayerIds.length }" @click="assignDevicePlayer(currentInputDevice,null)"><span class="assignment-radio"></span><span><strong>Unassigned</strong><small>Ignore this device for local gameplay.</small></span></button><button v-for="(player,index) in localAssignablePlayers" :key="player.id" data-focus class="device-assignment" :class="{ selected: currentInputDevice.assignedPlayerIds[0] === player.id }" @click="assignDevicePlayer(currentInputDevice,player.id)"><span class="assignment-radio"></span><span><strong>P{{ index+1 }} · {{ player.name }}</strong><small>{{ player.profile }} · {{ player.binds }}</small></span></button><div v-if="!localAssignablePlayers.length" class="muted">Add a local player before assigning devices.</div></div><div class="input-monitor"><div class="input-monitor-head"><span><strong>Live input explorer</strong><small>Move or press the physical control; highlighted values show what Gubsy recognizes.</small></span><b>{{ currentInputDevice.inputs.length }} INPUTS</b></div><button v-for="input in currentInputDevice.inputs" :key="input.label" data-focus class="input-monitor-row" :class="{ active: input.value !== 'UP' && !input.value.includes('0.00') && !input.value.includes('0.01') && !input.value.includes('0.02') && !input.value.includes('0.07') }" @click="pulseDeviceInput(input)"><span><strong>{{ input.label }}</strong><small>{{ input.kind }}</small></span><output>{{ input.value }}</output></button></div></div>
              </div>
            </template>

            <template v-else><div class="settings-layout"><div class="settings-list panel"><div class="panel-title"><div><p class="eyebrow">INPUT PROFILE</p><h3>Standard</h3></div><span class="save-indicator">{{ currentPlayer?.device }}</span></div><div v-for="item in [{key:'sensitivity',label:'Look sensitivity',note:'Horizontal and vertical camera speed.'},{key:'deadzone',label:'Stick deadzone',note:'Ignore small stick movement near center.'},{key:'vibration',label:'Vibration strength',note:'Controller rumble output.'}]" :key="item.key" class="setting-row"><div><strong>{{ item.label }}</strong><small>{{ item.note }}</small></div><label class="range-control"><input data-focus type="range" min="0" max="100" v-model.number="state.settings[item.key]"><output>{{ state.settings[item.key] }}%</output></label></div><div class="setting-row"><div><strong>Invert Y-axis</strong><small>Reverse vertical camera movement.</small></div><button data-focus class="toggle" :class="{ on: state.settings.invertY }" @click="state.settings.invertY = !state.settings.invertY"><span></span>{{ state.settings.invertY ? 'ON' : 'OFF' }}</button></div></div><aside class="preview-panel panel"><p class="eyebrow">DEVICE RESPONSE</p><div class="stick-preview"><span :style="{ transform: `translate(${state.settings.sensitivity/7}px, ${-state.settings.sensitivity/10}px)` }"></span></div><dl><dt>Device</dt><dd>{{ currentPlayer?.device }}</dd><dt>Deadzone</dt><dd>{{ state.settings.deadzone }}%</dd><dt>Vibration</dt><dd>{{ state.settings.vibration }}%</dd></dl></aside></div></template>
          </section>

          <section v-else-if="destination === 'saves'" class="screen progress-screen">
            <div class="toolbar"><div><p class="eyebrow">GAME-PROVIDED PROGRESSION</p><h3>{{ state.saves.length }} campaigns · automatic checkpoints</h3></div><button data-focus class="button primary" @click="sessionMode = 'New expedition'; chooseDestination('play'); playView = 'quest'">＋ Choose a new quest</button></div>
            <div class="saves-layout"><div class="save-list panel"><div class="panel-title"><div><p class="eyebrow">CAMPAIGNS</p><h3>Persistent progression</h3></div></div><button v-for="save in state.saves" :key="save.id" data-focus class="save-row" :class="{ selected: selectedSave === save.id }" @click="selectedSave = save.id"><span class="save-thumb"></span><span><strong>{{ save.name }}</strong><small>{{ save.owner }} · {{ save.detail }}</small></span><span><strong>{{ save.time }}</strong><small>{{ save.stamp }}</small></span><b :class="save.status">{{ save.status }}</b></button></div><div class="detail-panel panel progress-detail" v-if="currentSave"><p class="eyebrow">{{ currentSave.kind }} · {{ currentSave.status.toUpperCase() }}</p><h2>{{ currentSave.name }}</h2><div class="progress-owner"><span class="avatar" :style="{ '--avatar': state.profiles.find(profile => profile.name === currentSave.owner)?.color || '#a9f08b' }">{{ currentSave.owner.slice(0,2).toUpperCase() }}</span><span><small>ASSOCIATED PROFILE</small><strong>{{ currentSave.owner }}</strong><em>Ownership is fixed by this game's progression provider.</em></span></div><div class="save-manifest"><p class="eyebrow">RECORDED MOD SET · {{ currentSave.modManifest.length === 1 ? 'VANILLA' : `${currentSave.modManifest.length} PACKAGES` }}</p><div v-for="entry in currentSave.modManifest" :key="entry.name"><span>{{ entry.name }}</span><b>v{{ entry.version }}</b></div><small>The exact manifest is retained even when packages are not currently installed.</small></div><div class="checkpoint-list"><p class="eyebrow">CHECKPOINT HISTORY</p><div v-for="checkpoint in currentSave.checkpoints" :key="`${checkpoint.name}-${checkpoint.stamp}`" class="checkpoint-row"><span :class="{ current: checkpoint.current }"></span><div><strong>{{ checkpoint.name }}</strong><small>{{ checkpoint.stamp }}</small></div><b v-if="checkpoint.current">RESUME POINT</b></div></div><div class="action-row"><button data-focus class="button primary" :disabled="currentSave.status !== 'ready'" @click="state.activeProfile = currentSave.owner; chooseDestination('play'); notify(`${currentSave.name} selected with ${currentSave.owner}`)">Use in Play lobby</button><button data-focus class="button" @click="showModal('rename-save','Rename campaign',{ value:currentSave.name,payload:currentSave.id })">Rename</button><button data-focus class="button" @click="copySave(currentSave)">Copy</button><button data-focus class="button danger" @click="showModal('delete-save','Delete this campaign?',{ payload:currentSave.id,message:'The campaign and its checkpoints will be removed.',danger:true })">Delete</button></div><p v-if="currentSave.status !== 'ready'" class="warning">This progression was created by an incompatible game version. Keep it installed until a migration is available.</p></div></div>
          </section>

          <section v-else-if="destination === 'mods'" class="screen mods-screen">
            <div class="subnav"><button v-for="tab in ['Installed','Browse catalog']" :key="tab" data-focus :class="{ active: modsTab === tab }" @click="modsTab = tab">{{ tab }}</button><span class="bumper-hint"><kbd>Q</kbd><kbd>E</kbd> change section</span></div>
            <template v-if="modsTab === 'Installed'">
              <div class="toolbar"><div><p class="eyebrow">ON THIS DEVICE</p><h3>{{ state.mods.length }} installed</h3></div><button data-focus class="button" @click="notify('Installed mod folders rescanned')">↻ Refresh</button></div>
              <div class="mods-layout"><div class="mod-list panel" :class="{ 'mobile-pane-hidden': mobileLibraryDetail }"><button v-for="mod in state.mods" :key="mod.id" data-focus class="mod-row" :class="{ selected: selectedMod === mod.id }" @click="selectedMod = mod.id; mobileLibraryDetail = true"><span class="mod-thumb" :style="modThumbStyle(mod)"></span><span><strong>{{ mod.name }}</strong><small>by {{ mod.author }} · v{{ mod.version }}</small></span><b :class="{ warning: mod.status !== 'Compatible' }">{{ mod.status }}</b><span class="enabled-dot" :class="{ on: mod.sessionEnabled }"></span></button></div><div class="detail-panel panel" :class="{ 'mobile-pane-hidden': !mobileLibraryDetail }" v-if="currentMod"><button data-focus class="mobile-pane-back" @click="mobileLibraryDetail = false">‹ Installed mods</button><span class="mod-detail-art" :style="modThumbStyle(currentMod)"></span><p class="eyebrow">INSTALLED MOD</p><h2>{{ currentMod.name }}</h2><p>{{ currentMod.description }}</p><dl><dt>Author</dt><dd>{{ currentMod.author }}</dd><dt>Installed version</dt><dd><select data-focus v-model="currentMod.version"><option v-for="version in currentMod.versions" :key="version">{{ version }}</option></select></dd><dt>Used by lobby</dt><dd>{{ currentMod.sessionEnabled ? 'Yes' : 'No' }}</dd></dl><div class="relationship-block"><strong>Dependencies</strong><div v-if="!dependencyRows(currentMod).length" class="relationship-empty">None</div><div v-for="row in dependencyRows(currentMod)" :key="row.name" class="relationship-row"><span>{{ row.kind === 'Required' ? '↳' : '◇' }} {{ row.name }}</span><b>{{ row.kind }} · {{ row.installed ? 'installed' : 'missing' }}</b></div></div><div class="relationship-block"><strong>Required by</strong><div v-if="!dependentMods(currentMod.name).length" class="relationship-empty">No installed dependents</div><div v-for="mod in dependentMods(currentMod.name)" :key="mod.id" class="relationship-row"><span>↑ {{ mod.name }}</span><b>v{{ mod.version }}</b></div></div><p class="muted">This area manages files and versions on the device. Lobby activation lives in Play.</p><div class="action-row"><button v-if="currentMod.status === 'Needs update'" data-focus class="button primary" @click="updateMod(currentMod)">Update to {{ currentMod.latestVersion }}</button><button data-focus class="button danger" :disabled="currentMod.required" @click="requestUninstall(currentMod)">{{ currentMod.required ? 'Required by game' : 'Uninstall' }}</button></div><p v-if="currentMod.status !== 'Compatible'" class="warning">This installed version needs an update before it can be selected for a session. Update it here, or update-and-enable directly from Play → Session mods.</p></div></div>
            </template>
            <template v-else>
              <div class="toolbar catalog-toolbar"><label class="search"><span>⌕</span><input data-focus v-model="search" placeholder="Search the Gubsy mod catalog…"></label><button data-focus class="compatibility-filter" :class="{ active: compatibleModsOnly }" @click="toggleCompatibleMods"><span class="check-box" :class="{ checked: compatibleModsOnly }">{{ compatibleModsOnly ? '✓' : '' }}</span><span><strong>Compatible only</strong><small>Hide mods unusable on this build</small></span></button><button data-focus class="button" @click="notify('Catalog refreshed from local mod server')">↻ Refresh</button></div>
              <div class="mods-layout"><div class="mod-list panel" :class="{ 'mobile-pane-hidden': mobileLibraryDetail }"><div class="catalog-count">{{ filteredCatalog.length }} OF {{ state.modCatalog.length }} MODS</div><button v-for="mod in filteredCatalog" :key="mod.id" data-focus class="mod-row catalog-row" :class="{ selected: selectedCatalogMod === mod.id, incompatible: mod.installable === false }" @click="selectedCatalogMod = mod.id; mobileLibraryDetail = true"><span class="mod-thumb" :style="modThumbStyle(mod)"></span><span><strong>{{ mod.name }}</strong><small>{{ mod.category }} · by {{ mod.author }} · {{ mod.downloads }} downloads</small><em v-if="mod.installable === false">{{ mod.compatibility }}</em><em v-else-if="mod.dependencies?.length">Requires {{ mod.dependencies.join(', ') }}</em></span><b title="Community approval">★ {{ mod.rating }} positive</b><span class="enabled-dot" :class="{ on: state.mods.some(item => item.name === mod.name), bad: mod.installable === false }"></span></button><div v-if="!filteredCatalog.length" class="catalog-empty">No mods match this search and compatibility filter.</div></div><div class="detail-panel panel catalog-detail" :class="{ 'mobile-pane-hidden': !mobileLibraryDetail }" v-if="currentCatalogMod"><button data-focus class="mobile-pane-back" @click="mobileLibraryDetail = false">‹ Browse catalog</button><span class="mod-detail-art" :style="modThumbStyle(currentCatalogMod)"></span><div class="catalog-tags"><span>{{ currentCatalogMod.category }}</span><span v-for="tag in currentCatalogMod.tags" :key="tag">{{ tag }}</span></div><p class="eyebrow">CATALOG ENTRY</p><h2>{{ currentCatalogMod.name }}</h2><p>{{ currentCatalogMod.description }}</p><div class="compatibility-banner" :class="{ bad: currentCatalogMod.installable === false }"><span>{{ currentCatalogMod.installable === false ? '×' : '✓' }}</span><div><strong>{{ currentCatalogMod.installable === false ? 'Cannot install on this build' : 'Ready to install' }}</strong><small>{{ modCompatibility(currentCatalogMod) }}</small></div></div><div class="relationship-block"><strong>Dependency plan</strong><div v-if="!dependencyRows(currentCatalogMod).length" class="relationship-empty">No additional packages.</div><div v-for="row in dependencyRows(currentCatalogMod)" :key="`${row.kind}-${row.name}`" class="relationship-row"><span>{{ row.kind === 'Required' ? '↳' : '◇' }} {{ row.name }}</span><b>{{ row.kind }} · {{ row.installed ? 'installed' : 'auto-install' }}</b></div></div><dl><dt>Author</dt><dd>{{ currentCatalogMod.author }}</dd><dt>Version</dt><dd>{{ currentCatalogMod.version }}</dd><dt>Updated</dt><dd>{{ currentCatalogMod.updated }}</dd><dt>Download</dt><dd>{{ currentCatalogMod.size }}</dd><dt>Community approval</dt><dd>{{ currentCatalogMod.rating }} positive · {{ currentCatalogMod.downloads }} downloads</dd></dl><button data-focus class="button primary" :disabled="currentCatalogMod.installable === false || state.mods.some(item => item.name === currentCatalogMod.name)" @click="installCatalogMod(currentCatalogMod)">{{ state.mods.some(item => item.name === currentCatalogMod.name) ? 'Installed' : currentCatalogMod.installable === false ? 'Unavailable' : `Install${currentCatalogMod.dependencies?.length ? ' with dependencies' : ' mod'}` }}</button><p class="capture-note">This device manager installs packages. To install and immediately use one, browse from Play → Session mods.</p></div></div>
            </template>
          </section>
        </template>
      </section>
    </div>

    <footer class="prompt-bar"><div><kbd>↑↓←→</kbd><span>Navigate</span><kbd>Enter</kbd><span>Select</span><kbd>Esc</kbd><span>Back</span><kbd>Q / E</kbd><span>Local section</span></div><div><span class="focus-light"></span> Focus graph active</div></footer>

    <div v-if="modal" class="modal-layer" @mousedown.self="closeModal">
      <section class="modal" :class="{ wide: ['find-game','profile-history','disable-chain','uninstall-chain','runtime-mod-change','input-picker'].includes(modal.type) }" role="dialog" aria-modal="true">
        <button data-focus class="modal-close" @click="closeModal">×</button>
        <p class="eyebrow">{{ modal.type === 'capture' ? 'INPUT CAPTURE' : modal.type === 'pause' ? 'GAME SUSPENDED' : modal.danger ? 'CONFIRM ACTION' : 'CURRENT LOBBY' }}</p>
        <h2>{{ modal.title }}</h2>
        <p v-if="modal.message" class="modal-message">{{ modal.message }}</p>
        <input v-if="['new-profile','rename-profile','new-bind-profile','rename-bind-profile','rename-save'].includes(modal.type)" data-focus v-model="modal.value" class="modal-input" @keyup.enter="confirmModal">

        <div v-if="['disable-chain','uninstall-chain'].includes(modal.type)" class="lobby-modal-body">
          <div class="dependency-impact"><p class="eyebrow">AFFECTED PACKAGES</p><div v-for="name in (modal.type === 'disable-chain' ? [modal.payload.root, ...modal.payload.names] : modal.payload.names)" :key="name" class="relationship-row"><span>{{ name }}</span><b>{{ name === modal.payload.root || name === modal.payload.names?.[0] ? 'SELECTED' : 'DEPENDENT' }}</b></div></div>
          <div class="action-row modal-actions"><button data-focus class="button danger" @click="applyModChainAction">{{ modal.type === 'uninstall-chain' ? 'Remove affected set' : 'Disable affected set' }}</button><button data-focus class="button" @click="closeModal">Keep current set</button></div>
        </div>

        <div v-else-if="modal.type === 'runtime-mod-change'" class="lobby-modal-body">
          <div class="runtime-choice-summary"><span>{{ modal.payload.policy === 'next-stage' ? 'SAFE BOUNDARY RECOMMENDED' : 'NEW SESSION RECOMMENDED' }}</span><strong>{{ modal.payload.enable ? 'Adding content' : 'Removing content' }} from a running world</strong><p>Gubsy preserves the mod’s namespaced state by default. A game or mod can provide a migration and explicit cleanup operation.</p></div>
          <div class="runtime-actions"><button data-focus class="button primary" @click="applyRuntimeModChange('queue')">Apply at next stage</button><button data-focus class="button" @click="applyRuntimeModChange('now')">Apply now · preserve state</button><button v-if="!modal.payload.enable" data-focus class="button danger" @click="applyRuntimeModChange('clear')">Disable & clear mod state</button><button data-focus class="button" @click="closeModal">Cancel</button></div>
        </div>

        <div v-else-if="modal.type === 'profile-history' && modalProfile" class="lobby-modal-body">
          <div class="profile-history-head"><span class="avatar large-avatar" :style="{ '--avatar': modalProfile.color }">{{ modalProfile.name.slice(0,2).toUpperCase() }}</span><div><p class="eyebrow">PLAYER IDENTITY</p><h3>{{ modalProfile.name }}</h3><small>{{ modalProfile.played }} across every run and campaign</small></div></div>
          <div class="stat-grid"><div><span>RUNS</span><strong>{{ modalProfile.stats.runs }}</strong></div><div><span>WINS</span><strong>{{ modalProfile.stats.wins }}</strong></div><div><span>DEATHS</span><strong>{{ modalProfile.stats.deaths }}</strong></div><div><span>BEST SCORE</span><strong>{{ modalProfile.stats.score }}</strong></div><div><span>DEEPEST</span><strong>{{ modalProfile.stats.deepest }}</strong></div><div><span>CURRENCY</span><strong>{{ modalProfile.stats.currency }}</strong></div></div>
          <div class="profile-history-section"><p class="eyebrow">RECENT HISTORY</p><div class="history-row"><span>▶</span><div><strong>Temple 7-3 daily run</strong><small>Score 184,200 · 18m 42s · replay saved</small></div></div><div class="history-row"><span>✦</span><div><strong>First no-damage guardian</strong><small>Milestone earned 3 days ago</small></div></div><div class="history-row"><span>↻</span><div><strong>{{ modalProfile.stats.replays }} saved replays</strong><small>Game-provided run history associated with this profile</small></div></div></div>
          <div class="action-row modal-actions"><button data-focus class="button" @click="notify('Replay browser opened (demo)')">Browse replays</button><button data-focus class="button primary" @click="closeModal">Done</button></div>
        </div>

        <div v-else-if="modal.type === 'find-game'" class="lobby-modal-body">
          <p class="modal-message">Friend sessions and public games use the same join path. Connection details are chosen after compatibility checks.</p>
          <button v-for="(room,index) in state.rooms" :key="room.id" data-focus class="server-row" :class="{ disabled: room.status === 'Full' }" :disabled="room.status === 'Full'" @click="joinRoom(room)"><span><strong>{{ index === 0 ? '★ ' : '' }}{{ room.name }}</strong><small>{{ index === 0 ? `${room.host} is your friend` : `Hosted by ${room.host}` }} · {{ room.region }}</small></span><span>{{ room.players }}</span><span>{{ room.ping }} ms</span><b>{{ room.status === 'Full' ? 'FULL' : 'JOIN' }}</b></button>
          <div class="direct-join"><span><strong>Direct address</strong><small>Advanced fallback for LAN or self-hosted sessions</small></span><input data-focus value="192.168.1.42:24561"><button data-focus class="button" @click="onlineStatus = 'Connecting directly'; closeModal(); notify('Direct connection started')">Join</button></div>
          <div class="action-row modal-actions"><button data-focus class="button" @click="notify('Game list refreshed')">↻ Refresh</button><button data-focus class="button" @click="closeModal">Cancel</button></div>
        </div>

        <div v-else-if="modal.type === 'input-picker'" class="manual-input-picker"><label class="search"><span>⌕</span><input data-focus v-model="inputPickerSearch" placeholder="Search device, control, or input type…"></label><div class="manual-input-list"><button v-for="input in manualInputChoices" :key="`${input.deviceId}-${input.label}`" data-focus class="manual-input-row" @click="chooseManualInput(input)"><span><strong>{{ input.label }}</strong><small>{{ input.deviceName }} · {{ input.kind }}</small></span><b>{{ bindingLabelForInput(modal.payload.action,input).includes('→') ? bindingLabelForInput(modal.payload.action,input).split('→')[1] : 'DIRECT' }}</b></button></div><p class="capture-note">All recognized controls are available here, including unnamed joystick axes and HID buttons. Conversion defaults can be tuned after adding the binding.</p><div class="action-row modal-actions"><button data-focus class="button" @click="closeModal">Cancel</button></div></div>
        <div v-else-if="modal.type === 'capture'" class="capture-visual"><span></span><strong>Waiting for input…</strong><small>Press a button, move an axis, or turn a wheel. Gubsy will qualify noisy analog inputs before accepting one.</small></div>
        <div v-else-if="modal.type === 'pause'" class="pause-actions"><button data-focus class="button primary" @click="closeModal">Resume</button><button data-focus class="button" @click="closeModal(); playView = 'mods'; sessionModsTab = 'Current set'">Session mods</button><button data-focus class="button" @click="closeModal(); chooseDestination('settings')">Settings</button><button data-focus class="button danger" @click="sessionRunning = false; closeModal(); notify('Returned to title; queued runtime changes retained in lobby')">Return to title</button></div>
        <div v-else class="action-row modal-actions"><button data-focus class="button" :class="modal.danger ? 'danger' : 'primary'" @click="confirmModal">{{ modal.danger ? 'Confirm' : 'Continue' }}</button><button data-focus class="button" @click="capture = null; closeModal()">Cancel</button></div>
      </section>
    </div>
    <Transition name="toast"><div v-if="toast" class="toast"><span>✓</span>{{ toast }}</div></Transition>
  </main>
</template>
