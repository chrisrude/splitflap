# Fork overview and development notes

_Written 2026-10-29 against `main` @ `c2fa9f5`._

This repo is a fork of [scottbez1/splitflap](https://github.com/scottbez1/splitflap). It branched from
upstream commit `8c3520d` ("Slack -> Discord", 2023-09) and has 32 local commits (2023-09-11 → 2023-10-08).
The firmware at `HEAD` is what runs on the assembled display.

Related notes:
- [02-fork-review.md](02-fork-review.md): issues found in the local changes
- [03-upstream-review.md](03-upstream-review.md): what changed upstream and whether to take it

## 1. The physical build

- **Layout:** 2 rows × 12 characters = 24 modules (`-DNUM_MODULES=24`).
- **Flaps:** 40-flap "classic" set: `' '`, `a–z`, `0–9`, `.`, `,`, `'` (`arduino/splitflap/Splitflap/config.h`).
  The firmware uses lowercase letters, and the web UI renders them in uppercase.
- **Drivers:** Chainlink Driver boards (6 modules each, so 4 boards), built with `-DCHAINLINK`.
- **Controller:** LilyGO **T-Display-S3** (ESP32-S3, 16 MB flash, OPI PSRAM, 170×320 parallel ST7789 LCD).
  Upstream targets the original ESP32 T-Display, so this is a port.
- **Module wiring order (inferred from the HTTP handler):** the chain starts at the **top-right** module and runs
  right-to-left across the top row (modules 0–11). It then continues left-to-right across the bottom row
  (modules 12–23). The firmware compensates for this only in `http_server_task.cpp` (see the review).
- **Reset button:** external button on GPIO 43, active-low with internal pull-up (EasyButton defaults).

### Controller pinout as built

| Function | Macro in `spi_io_config.h` | GPIO |
|---|---|---|
| Shift-register latch | `LATCH_PIN` = `MISO` | 13 |
| SPI MISO (sensor data in) | `PIN_NUM_MISO` = `SS` | 10 |
| SPI MOSI (motor data out) | `PIN_NUM_MOSI` = `MOSI` | 11 |
| SPI CLK | `PIN_NUM_CLK` = `SCK` | 12 |
| Reset button | `RESET_BUTTON_PIN` (main.cpp) | 43 (U0TXD on the header) |
| LCD | parallel 8-bit, see `platformio.ini` | 39–42, 45–48, 5–9, BL 38 |

The macros reuse the Arduino SPI pin *names* for other jobs (e.g. latch = "MISO"), which is confusing. Prefer
literal GPIO numbers. The pinout comment in `config.h` still lists the classic ESP32 pins (25/33/39/32) and is
wrong for this build.

I couldn't confirm these from the repo; they're worth writing down: which mechanical revision the enclosure is
(v0.5/v0.7 laser-cut?), sensor board type, PSU, and how the Chainlink Driver chain connects to the S3 (custom
wiring, since the `chainlinkBuddyTDisplay` board is for the original T-Display pinout).

## 2. Repository layout (as of HEAD)

```
3d/                      OpenSCAD enclosure/flap design + render scripts (upstream, unchanged)
electronics/             KiCad projects: chainlinkDriver, chainlinkBuddy*, chainlinkBase, sensor, classic
                         (only KiCad-7 "rescue"/sym-lib-table churn locally, no real design changes)
arduino/splitflap/       PlatformIO firmware project  ← most local work is here
  platformio.ini         envs: chainlink (production), chainlinkDebug
  Splitflap/             src_dir: config.h, src/splitflap_module.h, src/spi_io_config.h
                         (Splitflap.ino is now a dead "hello world" stub, excluded from the build)
  esp32/core/            SplitflapTask (motor control), Task<> CRTP wrapper, logger, uart_stream (unused now)
  esp32/splitflap/       main.cpp, DisplayTask (LCD), SerialTask (+ legacy JSON & protobuf protocols),
                         http_server_task.* + network.h  ← NEW (web server)
                         http_task/mqtt_task (upstream, compiled out)
  data/web/              built web UI, uploaded to SPIFFS (committed build output)
  lib/json11             JSON lib used by the HTTP server
software/frontend/splitweb/   SvelteKit web UI source  ← NEW
software/chainlink/      upstream Python/JS host libraries for the serial protobuf protocol
proto/                   splitflap.proto (serial protocol schema)
```

## 3. Firmware architecture

FreeRTOS tasks are created in `esp32/splitflap/main.cpp:setup()`. The Arduino `loopTask` is then deleted.

| Task | Core | Stack | Role |
|---|---|---|---|
| `Splitflap` (SplitflapTask) | 1 | 16 KB | Runs the stepper/sensor loop over SPI shift registers; consumes a command queue (depth 5) |
| `Display` (DisplayTask) | 0 | 6 KB | Draws module states plus 3 message lines on the LCD |
| `Serial` (SerialTask) | 0 | 16 KB | USB-CDC `Serial`: legacy JSON/plaintext protocol, switches to protobuf on demand; drains the log queue |
| `HTTP_Server` (HTTPServerTask) | 0 | 32 KB | Connects WiFi, mounts SPIFFS, starts ESPAsyncWebServer, then loops every 10 s for reconnects |
| AsyncTCP (library) | 0 | — | Runs the HTTP request handlers (`CONFIG_ASYNC_TCP_RUNNING_CORE=0`) |

Core 1 is reserved for the motor loop. Other code talks to it only through `SplitflapTask`'s queue methods
(`showString`, `resetAll`, …) and `getState()`.

### HTTP API (`http_server_task.cpp`)

| Method | Path | Behaviour |
|---|---|---|
| GET | `/` | Static files from SPIFFS `/web/`, default `index.html` |
| GET | `/text` | Last text set **via HTTP** (not read from the modules) |
| POST | `/set` | Form field `message`. Padded/truncated to 24, lowercased, unknown chars → space. First row reversed for wiring. Echoes the sanitized text |
| GET | `/flaps` | Character set as a plain string |
| GET | `/status` | JSON array of per-module `{state, flap_index, moving, home_state, count_unexpected_home, count_missed_home}` |
| POST | `/reset` | Re-home all modules |

There is no auth, and there's no mDNS. The DHCP hostname is `splitflap-xxxxx`, built from the MAC address.

### Web UI (`software/frontend/splitweb`)

- SvelteKit 1 + Svelte 4, `adapter-static` with SPA fallback, TypeScript.
- `src/lib/split_api.ts` wraps the HTTP API. In `vite dev`, failed requests fall back to mock data, so you can
  develop the UI without hardware (`npm run dev`).
- `src/routes/+page.svelte`: a hidden text input drives the on-screen board. Typed characters preview as
  "pending" flaps (orange), and Enter submits. Shift+Enter/Tab jumps to the next row. Ctrl+key fills the whole
  board with that key. Keys not in the character set are ignored.
- `Flap.svelte` shows a red/orange corner marker for error/warning state, with a hover table of module status.
- Layout constants are hard-coded in `src/lib/constants.ts` (`NUM_MODULES=24`, `MODULES_PER_ROW=12`, 40 flaps).

## 4. Build & deploy

```sh
# one-time
source .venv/bin/activate                     # provides `pio` (.envrc does this under direnv)
cp arduino/splitflap/esp32/splitflap/secrets.h.example arduino/splitflap/esp32/splitflap/secrets.h
#   edit WIFI_SSID / WIFI_PASSWORD (secrets.h is gitignored; the build fails without it)

# firmware
cd arduino/splitflap
pio run -e chainlink -t upload                # USB-CDC
pio device monitor -e chainlink

# web UI
cd software/frontend/splitweb
npm install
npm run build                                 # vite build + scripts/rename.js → build_shortened/
rm -rf ../../../arduino/splitflap/data/web && cp -r build_shortened ../../../arduino/splitflap/data/web
cd ../../../arduino/splitflap && pio run -e chainlink -t uploadfs   # SPIFFS image of data/
```

Why `rename.js`: SPIFFS limits paths to about 32 characters, so the script shortens SvelteKit's
`_app/immutable/...` paths and rewrites the references inside the files.

**Verified 2026-10-29:** `pio run -e chainlink` at HEAD succeeds once `secrets.h` exists (RAM 21.7%, flash
16.3%). Because the platform and TFT_eSPI are unpinned, it resolved to espressif32 **7.1.3** (Arduino-ESP32
**2.0.17**), TFT_eSPI **2.5.43**, ESPAsyncWebServer-esphome **3.4.1**, AsyncTCP-esphome **2.1.4**, and
EasyButton **2.0.3**. Those are probably not the versions the device was flashed with in 2023, so pin them
(see the review).

The `.github/workflows/pio.yml` CI still builds envs that no longer exist (`uno-*`, `esp32`, `chainlinkBase`, …),
so it fails.

## 5. Notes for future development

Roughly in priority order. The review has the bug list. These are mostly direction and structure.

1. **Fix the review's "high" items first.** Two of them are user-visible: the reset button crashes the device,
   and the web Reset button writes "reset" on the board.
2. **Pin the toolchain** (`platform = espressif32@<x>`, `bodmer/TFT_eSPI @ 2.5.43`, etc.) and record the
   known-good versions. Add a CI job that builds `env:chainlink` (copying `secrets.h.example`) and the web UI.
3. **One source of truth for the physical layout.** Put a `getLayoutPosition()` that matches the real wiring
   (top row reversed, `DISPLAY_COLUMNS 12`) in `display_layouts.h`. Use it for the LCD preview, and add a
   "logical text → module index" mapping inside `SplitflapTask::showString` (or a wrapper) so HTTP, serial,
   and MQTT all behave the same. Expose rows/cols in the API (e.g. `GET /config`) and have the web UI read
   them, which removes the constants in `constants.ts`.
4. **Report real state.** Build `/text` from `getState()` (flap index → `flaps[]`), and push state changes to the
   browser with `AsyncWebSocket`/SSE (both are in ESPAsyncWebServer) instead of manual "Refresh" buttons.
5. **Filesystem:** move from SPIFFS (deprecated) to LittleFS or FFat. That removes the 32-char path limit and
   makes `rename.js` unnecessary. Enable `precompress` in adapter-static so `serveStatic` serves `.gz`
   automatically. Pick a partition table for 16 MB that also leaves room for upstream's persistent calibration
   (FFat) if you adopt it (see the upstream review).
6. **Network niceties:** `MDNS.begin(HOSTNAME)` so `http://splitflap.local` works. Put SSID/IP on the LCD
   message line. Make WiFi failure non-fatal. Optionally add a fallback AP/captive-portal for WiFi setup.
7. **OTA updates.** The display is assembled, so USB access is presumably awkward. ArduinoOTA is a few lines
   (upstream has a working example in its MQTT task). An authenticated `/update` endpoint that also accepts a
   filesystem image would let the web UI be updated wirelessly too.
8. **Calibration without opening the case.** Upstream added per-module software offsets persisted to flash, and
   it's the most useful upstream firmware feature for an already-built display. A small calibration page in
   the Svelte UI (per-module +0.1/+0.5 flap, "set home", "save") would fit naturally.
9. **Automation hooks:** MQTT + Home Assistant discovery (upstream has this) or a simple REST "scheduled
   messages / clock mode" on the device.
10. **Security:** any device on the LAN can change the text or trigger homing. That's fine on a home network,
    but if it is ever exposed, add a shared-secret header or basic auth on POST routes.
11. **Housekeeping:** delete dead code (`Splitflap.ino` stub, `uart_stream` if unused, AVR remnants,
    commented-out `HTTP`/`MQTT` blocks), drop KiCad rescue-file churn, replace the template `README.md` in
    `splitweb`, decide on the license (`splitweb/package.json` says MIT while the repo is Apache-2.0), and add
    `build_shortened/` to the web `.gitignore`.
12. **Tests:** the Playwright/Vitest setup is template-only. A Vitest test for row-wrapping/`addSpaces` and a
    Playwright smoke test against the dev mocks would be cheap. On the firmware side, `sanitize_string` and the
    layout mapping are pure functions that could get native (`pio test -e native`) unit tests.
