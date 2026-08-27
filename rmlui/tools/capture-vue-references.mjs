import { chromium } from 'playwright-core'
import { mkdir } from 'node:fs/promises'
import { dirname, resolve } from 'node:path'
import { fileURLToPath } from 'node:url'

const here = dirname(fileURLToPath(import.meta.url))
const outputRoot = resolve(here, '../docs/vue-reference')
const baseUrl = process.env.GUBSY_VUE_URL || 'http://127.0.0.1:4173'
const executablePath = process.env.CHROME_PATH || '/usr/bin/google-chrome'

const sizes = [
  ['1920x1080', 1920, 1080],
  ['1280x720', 1280, 720],
  ['412x915', 412, 915],
  ['915x412', 915, 412],
]

const nav = (page, label) =>
  page.locator('button.nav-item').filter({ hasText: label }).first().click()

const views = [
  ['play-lobby', async () => {}],
  ['play-quest', async page =>
    page.locator('button.lobby-option.command').filter({ hasText: 'Resume point' }).click()],
  ['play-settings', async page =>
    page.locator('button.lobby-option.command').filter({ hasText: 'Expedition rules' }).click()],
  ['play-session-mods', async page =>
    page.locator('button.lobby-option.command').filter({ hasText: 'Session mods' }).click()],
  ['players', async page => nav(page, 'Players')],
  ['players-profiles', async page => {
    await nav(page, 'Players')
    await page.locator('.subnav button').filter({ hasText: /^Profiles$/ }).click()
  }],
  ['players-devices', async page => {
    await nav(page, 'Players')
    await page.locator('.subnav button').filter({ hasText: /^Devices$/ }).click()
  }],
  ['settings', async page => nav(page, 'Settings')],
  ...['Audio', 'Accessibility', 'Gameplay'].map(tab => [
    `settings-${tab.toLowerCase()}`,
    async page => {
      await nav(page, 'Settings')
      await page.locator('.subnav button').filter({ hasText: new RegExp(`^${tab}$`) }).click()
    },
  ]),
  ['controls-bindings', async page => nav(page, 'Controls')],
  ['controls-devices', async page => {
    await nav(page, 'Controls')
    await page.locator('.subnav button').filter({ hasText: /^Devices$/ }).click()
  }],
  ['controls-input-tuning', async page => {
    await nav(page, 'Controls')
    await page.locator('.subnav button').filter({ hasText: /^Input tuning$/ }).click()
  }],
  ['progress', async page => nav(page, 'Progress')],
  ['mods-installed', async page => nav(page, 'Mods')],
  ['mods-catalog', async page => {
    await nav(page, 'Mods')
    await page.locator('.subnav button').filter({ hasText: 'Browse catalog' }).click()
  }],
]

const browser = await chromium.launch({ headless: true, executablePath })
const page = await browser.newPage()
let count = 0

try {
  for (const [sizeName, width, height] of sizes) {
    const outputDirectory = resolve(outputRoot, sizeName)
    await mkdir(outputDirectory, { recursive: true })
    await page.setViewportSize({ width, height })

    for (const [viewName, navigate] of views) {
      await page.goto(baseUrl, { waitUntil: 'networkidle' })
      await page.evaluate(() => localStorage.removeItem('gubsy-ui-demo-state'))
      await page.reload({ waitUntil: 'networkidle' })
      await navigate(page)
      await page.waitForTimeout(100)
      await page.screenshot({
        path: resolve(outputDirectory, `${viewName}.jpg`),
        type: 'jpeg',
        quality: 88,
      })
      count += 1
    }
  }
} finally {
  await browser.close()
}

console.log(`Captured ${count} Vue references in ${outputRoot}`)
