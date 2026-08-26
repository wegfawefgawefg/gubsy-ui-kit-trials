<script setup>
import { computed, nextTick, onMounted, onUnmounted, reactive, ref, watch } from 'vue'
import { initialState, navItems, settingGroups } from './demoData'

const clone = (value) => JSON.parse(JSON.stringify(value))
const saved = localStorage.getItem('gubsy-ui-demo-state')
const state = reactive(saved ? { ...clone(initialState), ...JSON.parse(saved) } : clone(initialState))

const destination = ref('play')
const playerTab = ref('Local players')
const onlineTab = ref('Lobby')
const settingsTab = ref('Display')
const controlTab = ref('Bindings')
const saveTab = ref('Continue')
const selectedPlayer = ref(0)
const selectedAction = ref('Menu Up')
const selectedSave = ref(state.saves[0]?.id)
const selectedMod = ref(state.mods[0]?.id)
const selectedRoom = ref(state.rooms[0]?.id)
const scenario = ref('Populated')
const onlineStatus = ref('Offline')
const modal = ref(null)
const toast = ref('')
const inputMode = ref('controller')
const capture = ref(null)
const search = ref('')
let toastTimer

const currentProfile = computed(() => state.profiles.find((item) => item.name === state.activeProfile) || state.profiles[0])
const currentPlayer = computed(() => state.players[selectedPlayer.value] || state.players[0])
const currentBindProfile = computed(() => state.bindProfiles.find((item) => item.name === state.bindProfile) || state.bindProfiles[0])
const currentSave = computed(() => state.saves.find((item) => item.id === selectedSave.value) || state.saves[0])
const currentMod = computed(() => state.mods.find((item) => item.id === selectedMod.value) || state.mods[0])
const currentRoom = computed(() => state.rooms.find((item) => item.id === selectedRoom.value) || state.rooms[0])
const filteredBindings = computed(() => Object.entries(state.bindings).filter(([name]) => name.toLowerCase().includes(search.value.toLowerCase())))
const sectionTitle = computed(() => navItems.find((item) => item.id === destination.value)?.label || 'Menu')

watch(state, () => localStorage.setItem('gubsy-ui-demo-state', JSON.stringify(state)), { deep: true })

function notify(message) {
  toast.value = message
  clearTimeout(toastTimer)
  toastTimer = setTimeout(() => { toast.value = '' }, 2600)
}

function chooseDestination(id) {
  destination.value = id
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
  } else if (item.type === 'new-game') {
    const id = Date.now()
    state.saves.unshift({ id, name: name || 'New Expedition', detail: 'Green Valley · 0%', time: '0m', status: 'ready', stamp: 'Just now' })
    selectedSave.value = id
    notify('New expedition created')
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

function installSampleMod() {
  const existing = state.mods.find((mod) => mod.name === 'Wind Tunnels')
  if (existing) return notify('No new compatible mods found')
  const mod = { id: Date.now(), name: 'Wind Tunnels', author: 'Cicada', version: '1.0.0', enabled: false, status: 'Compatible', description: 'Adds optional wind-current challenge rooms.' }
  state.mods.push(mod)
  selectedMod.value = mod.id
  notify('Wind Tunnels installed')
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

function captureInput(event) {
  if (!capture.value || event.key === 'Escape') return
  event.preventDefault()
  const label = event.key.length === 1 ? `Keyboard ${event.key.toUpperCase()}` : `Keyboard ${event.key}`
  const list = state.bindings[capture.value.action]
  if (capture.value.slot === null) list.push(label)
  else list[capture.value.slot] = label
  capture.value = null
  modal.value = null
  selectedAction.value = capture.value?.action || selectedAction.value
  notify(`Bound ${label}`)
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
    onlineTab.value = 'Lobby'
    notify(kind === 'local' ? 'Local lobby ready' : 'Room VIOLET-7 is live')
  }, 850)
}

function joinRoom(room) {
  if (room.status === 'Full') return notify('That room is full')
  onlineStatus.value = 'Connecting'
  setTimeout(() => {
    onlineStatus.value = 'Client · Waiting for host'
    onlineTab.value = 'Lobby'
    notify(`Joined ${room.name}`)
  }, 850)
}

function setScenario(value) {
  scenario.value = value
  if (value === 'Loading') setTimeout(() => { if (scenario.value === 'Loading') scenario.value = 'Populated' }, 1500)
}

function localPage(direction) {
  const tabs = destination.value === 'settings'
    ? Object.keys(settingGroups)
    : destination.value === 'controls'
      ? ['Bindings', 'Input tuning']
      : destination.value === 'online'
        ? ['Lobby', 'Host', 'Join', 'Browse']
        : destination.value === 'players'
          ? ['Local players', 'Profiles', 'Devices']
          : []
  const current = destination.value === 'settings' ? settingsTab : destination.value === 'controls' ? controlTab : destination.value === 'online' ? onlineTab : playerTab
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
            <div class="hero-card">
              <div class="hero-copy"><p class="eyebrow">LAST PLAYED · TODAY</p><h2>The Glass Caverns</h2><p>Temple Depths · 42% complete · 12h 48m</p><div class="action-row"><button data-focus class="button primary large" @click="notify('Continue command sent to game')">▶ Continue</button><button data-focus class="button" @click="chooseDestination('saves')">Choose save</button></div></div>
              <div class="hero-progress"><span>42%</span><small>EXPEDITION</small></div>
            </div>
            <div class="card-grid three">
              <button data-focus class="feature-card" @click="showModal('new-game', 'Begin a new expedition', { value: 'New Expedition', message: 'Name this save slot. Your existing saves are safe.' })"><span class="feature-icon">＋</span><strong>New game</strong><small>Start a fresh expedition</small></button>
              <button data-focus class="feature-card" @click="chooseDestination('players')"><span class="feature-icon">◆</span><strong>Local co-op</strong><small>{{ state.players.length }} of 4 local players</small></button>
              <button data-focus class="feature-card" @click="chooseDestination('online')"><span class="feature-icon">◎</span><strong>Online play</strong><small>Host or join a room</small></button>
            </div>
            <div class="recent-panel panel"><div class="panel-title"><div><p class="eyebrow">SESSION READY</p><h3>Before you descend</h3></div><button data-focus class="button compact" @click="showModal('pause','Game paused',{ message: 'This previews the in-game menu context without pretending the browser mock is running gameplay.' })">Pause-menu preview</button></div><div class="summary-line"><span>Profile</span><strong>{{ state.activeProfile }}</strong><span>Bindings</span><strong>{{ state.bindProfile }}</strong><span>Players</span><strong>{{ state.players.filter(p => p.ready).length }} ready</strong></div></div>
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
              <div class="card-grid three"><article v-for="profile in state.profiles" :key="profile.id" class="profile-tile" :class="{ selected: state.activeProfile === profile.name }"><button data-focus class="tile-main" @click="state.activeProfile = profile.name; notify(`${profile.name} is now active`)"><span class="avatar large-avatar" :style="{ '--avatar': profile.color }">{{ profile.name.slice(0,2).toUpperCase() }}</span><strong>{{ profile.name }}</strong><small>{{ profile.played }} played</small><span class="selected-label">{{ state.activeProfile === profile.name ? 'ACTIVE' : 'SELECT' }}</span></button><div class="tile-actions"><button data-focus @click="showModal('rename-profile','Rename profile',{ value: profile.name, payload: profile.id })">Rename</button><button data-focus @click="showModal('reset-profile','Reset profile preferences?',{ payload: profile.id, message: 'Settings and assignments return to defaults. Save progress is preserved.', danger: true })">Reset</button><button data-focus :disabled="state.profiles.length <= 1" @click="showModal('delete-profile','Delete profile?',{ payload: profile.id, message: `Delete ${profile.name}? Save data will not be removed.`, danger: true })">Delete</button></div></article></div>
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
            <div class="subnav"><button v-for="tab in ['Bindings','Input tuning']" :key="tab" data-focus :class="{ active: controlTab === tab }" @click="controlTab = tab">{{ tab }}</button><span class="bumper-hint"><kbd>Q</kbd><kbd>E</kbd> change section</span></div>
            <template v-if="controlTab === 'Bindings'"><div class="toolbar controls-toolbar"><label class="search"><span>⌕</span><input data-focus v-model="search" placeholder="Filter actions…"></label><select data-focus v-model="state.bindProfile"><option v-for="profile in state.bindProfiles" :key="profile.id">{{ profile.name }}</option></select><div class="action-row bind-profile-actions"><button data-focus class="button compact" @click="showModal('new-bind-profile','New binding profile',{ value: 'Custom Binds' })">＋ New</button><button data-focus class="button compact" @click="showModal('rename-bind-profile','Rename binding profile',{ value: currentBindProfile?.name, payload: currentBindProfile?.id })">Rename</button><button data-focus class="button compact" @click="showModal('reset-bind-profile','Reset all bindings?',{ message: 'Registered actions return to their default keyboard and controller bindings.', danger: true })">Reset</button><button data-focus class="button compact danger" :disabled="state.bindProfiles.length <= 1" @click="showModal('delete-bind-profile','Delete binding profile?',{ payload: currentBindProfile?.id, message: 'Players using it will fall back to the first available profile.', danger: true })">Delete</button></div></div><div class="bindings-layout"><div class="binding-list panel"><button v-for="([action, binds]) in filteredBindings" :key="action" data-focus class="binding-row" :class="{ selected: selectedAction === action }" @click="selectedAction = action"><span><strong>{{ action }}</strong><small>{{ ['Move','Look'].includes(action) ? 'Analog action' : 'Button action' }}</small></span><span class="bind-summary">{{ binds[0] || 'UNBOUND' }}</span></button></div><div class="detail-panel panel"><p class="eyebrow">SELECTED ACTION</p><h2>{{ selectedAction }}</h2><p class="muted">Each action can have multiple device bindings.</p><div class="bind-slots"><div v-for="(binding,index) in state.bindings[selectedAction]" :key="`${binding}-${index}`" class="bind-slot"><span>{{ index+1 }}</span><strong>{{ binding }}</strong><button data-focus @click="beginCapture(selectedAction,index)">Replace</button><button data-focus class="remove" @click="removeBinding(selectedAction,index)">×</button></div></div><button data-focus class="button primary" @click="beginCapture(selectedAction)">＋ Add binding</button><p class="capture-note">Button, keyboard, 1D axis, and 2D stick capture are represented by this flow. The native version will apply deadzone/noise qualification.</p></div></div></template>
            <template v-else><div class="settings-layout"><div class="settings-list panel"><div class="panel-title"><div><p class="eyebrow">INPUT PROFILE</p><h3>Standard</h3></div><span class="save-indicator">{{ currentPlayer?.device }}</span></div><div v-for="item in [{key:'sensitivity',label:'Look sensitivity',note:'Horizontal and vertical camera speed.'},{key:'deadzone',label:'Stick deadzone',note:'Ignore small stick movement near center.'},{key:'vibration',label:'Vibration strength',note:'Controller rumble output.'}]" :key="item.key" class="setting-row"><div><strong>{{ item.label }}</strong><small>{{ item.note }}</small></div><label class="range-control"><input data-focus type="range" min="0" max="100" v-model.number="state.settings[item.key]"><output>{{ state.settings[item.key] }}%</output></label></div><div class="setting-row"><div><strong>Invert Y-axis</strong><small>Reverse vertical camera movement.</small></div><button data-focus class="toggle" :class="{ on: state.settings.invertY }" @click="state.settings.invertY = !state.settings.invertY"><span></span>{{ state.settings.invertY ? 'ON' : 'OFF' }}</button></div></div><aside class="preview-panel panel"><p class="eyebrow">DEVICE RESPONSE</p><div class="stick-preview"><span :style="{ transform: `translate(${state.settings.sensitivity/7}px, ${-state.settings.sensitivity/10}px)` }"></span></div><dl><dt>Device</dt><dd>{{ currentPlayer?.device }}</dd><dt>Deadzone</dt><dd>{{ state.settings.deadzone }}%</dd><dt>Vibration</dt><dd>{{ state.settings.vibration }}%</dd></dl></aside></div></template>
          </section>

          <section v-else-if="destination === 'saves'" class="screen">
            <div class="subnav"><button v-for="tab in ['Continue','Manage saves']" :key="tab" data-focus :class="{ active: saveTab === tab }" @click="saveTab = tab">{{ tab }}</button></div>
            <div class="saves-layout"><div class="save-list panel"><div class="panel-title"><div><p class="eyebrow">SAVE CATALOG</p><h3>{{ state.saves.length }} expeditions</h3></div><button data-focus class="icon-button" @click="showModal('new-game','Begin a new expedition',{ value:'New Expedition' })">＋</button></div><button v-for="save in state.saves" :key="save.id" data-focus class="save-row" :class="{ selected: selectedSave === save.id }" @click="selectedSave = save.id"><span class="save-thumb"></span><span><strong>{{ save.name }}</strong><small>{{ save.detail }}</small></span><span><strong>{{ save.time }}</strong><small>{{ save.stamp }}</small></span><b :class="save.status">{{ save.status }}</b></button></div><div class="detail-panel panel" v-if="currentSave"><span class="save-hero"></span><p class="eyebrow">{{ currentSave.status.toUpperCase() }}</p><h2>{{ currentSave.name }}</h2><p>{{ currentSave.detail }} · {{ currentSave.time }}</p><div class="action-row"><button data-focus class="button primary" :disabled="currentSave.status !== 'ready'" @click="notify(`Loading ${currentSave.name}…`)">▶ Load</button><button data-focus class="button" @click="showModal('rename-save','Rename save',{ value:currentSave.name,payload:currentSave.id })">Rename</button><button data-focus class="button" @click="copySave(currentSave)">Copy</button><button data-focus class="button danger" @click="showModal('delete-save','Delete this save?',{ payload:currentSave.id,message:'This operation cannot be undone.',danger:true })">Delete</button></div><p v-if="currentSave.status !== 'ready'" class="warning">This save was created by an incompatible game version. Keep it installed until a migration is available.</p></div></div>
          </section>

          <section v-else-if="destination === 'mods'" class="screen">
            <div class="toolbar"><div><p class="eyebrow">INSTALLED CONTENT</p><h3>{{ state.mods.filter(m => m.enabled).length }} enabled · {{ state.mods.length }} installed</h3></div><div class="action-row"><button data-focus class="button" @click="notify('Mod folders rescanned')">↻ Refresh</button><button data-focus class="button primary" @click="installSampleMod">Browse / install sample</button></div></div><div class="mods-layout"><div class="mod-list panel"><button v-for="mod in state.mods" :key="mod.id" data-focus class="mod-row" :class="{ selected: selectedMod === mod.id }" @click="selectedMod = mod.id"><span class="mod-icon">⬡</span><span><strong>{{ mod.name }}</strong><small>by {{ mod.author }} · v{{ mod.version }}</small></span><b :class="{ warning: mod.status !== 'Compatible' }">{{ mod.status }}</b><span class="enabled-dot" :class="{ on: mod.enabled }"></span></button></div><div class="detail-panel panel" v-if="currentMod"><p class="eyebrow">MOD DETAILS</p><h2>{{ currentMod.name }}</h2><p>{{ currentMod.description }}</p><dl><dt>Author</dt><dd>{{ currentMod.author }}</dd><dt>Version</dt><dd>{{ currentMod.version }}</dd><dt>Status</dt><dd>{{ currentMod.status }}</dd></dl><div class="action-row"><button data-focus class="button" :class="{ primary: !currentMod.enabled }" :disabled="currentMod.status !== 'Compatible'" @click="currentMod.enabled = !currentMod.enabled; notify('Mod list changed; restart may be required')">{{ currentMod.enabled ? 'Disable mod' : 'Enable mod' }}</button><button data-focus class="button danger" @click="state.mods = state.mods.filter(mod => mod.id !== currentMod.id); selectedMod = state.mods[0]?.id; notify('Mod removed')">Remove</button></div><p v-if="currentMod.status !== 'Compatible'" class="warning">This mod cannot be enabled until its compatibility metadata is updated.</p></div></div>
          </section>
        </template>
      </section>
    </div>

    <footer class="prompt-bar"><div><kbd>↑↓←→</kbd><span>Navigate</span><kbd>Enter</kbd><span>Select</span><kbd>Esc</kbd><span>Back</span><kbd>Q / E</kbd><span>Local section</span></div><div><span class="focus-light"></span> Focus graph active</div></footer>

    <div v-if="modal" class="modal-layer" @mousedown.self="closeModal"><section class="modal" role="dialog" aria-modal="true"><button data-focus class="modal-close" @click="closeModal">×</button><p class="eyebrow">{{ modal.type === 'capture' ? 'INPUT CAPTURE' : modal.type === 'pause' ? 'GAME SUSPENDED' : modal.danger ? 'CONFIRM ACTION' : 'GUBSY SHELL' }}</p><h2>{{ modal.title }}</h2><p v-if="modal.message" class="modal-message">{{ modal.message }}</p><input v-if="['new-profile','rename-profile','new-bind-profile','rename-bind-profile','rename-save','new-game'].includes(modal.type)" data-focus v-model="modal.value" class="modal-input" @keyup.enter="confirmModal"><div v-if="modal.type === 'capture'" class="capture-visual"><span></span><strong>Waiting for input…</strong><small>Axis movement will require a sustained threshold in native Gubsy.</small></div><div v-if="modal.type === 'pause'" class="pause-actions"><button data-focus class="button primary" @click="closeModal">Resume</button><button data-focus class="button" @click="closeModal(); chooseDestination('settings')">Settings</button><button data-focus class="button danger" @click="closeModal(); notify('Return-to-title command sent')">Return to title</button></div><div v-else class="action-row modal-actions"><button v-if="modal.type !== 'capture'" data-focus class="button" :class="modal.danger ? 'danger' : 'primary'" @click="confirmModal">{{ modal.danger ? 'Confirm' : 'Continue' }}</button><button data-focus class="button" @click="capture = null; closeModal()">Cancel</button></div></section></div>
    <Transition name="toast"><div v-if="toast" class="toast"><span>✓</span>{{ toast }}</div></Transition>
  </main>
</template>
