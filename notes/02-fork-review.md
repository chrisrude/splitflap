# Review of local changes since the fork

_Scope: `git diff 8c3520d..c2fa9f5` (32 commits, 65 files, +6540/−768). About 4.4k of those lines are
`package-lock.json` and built web assets. Reviewed 2026-10-29._

Paths are relative to the repo root. `fw/` = `arduino/splitflap/`, `web/` = `software/frontend/splitweb/`.

What the fork changed:
1. Ported the firmware from the original ESP32 T-Display to the **LilyGO T-Display-S3**: platformio board,
   parallel LCD pins, SPI pins, `SPI2_HOST`, UART config, and USB-CDC `Serial` instead of `UartStream`.
2. Stripped AVR/ESP8266/Arduino-IDE support from `config.h`, `spi_io_config.h`, and `platformio.ini`.
3. Added a **reset button** (EasyButton, GPIO 43).
4. Added an **HTTP server** (`http_server_task.*`, `network.h`): ESPAsyncWebServer serving a static UI from
   SPIFFS, plus a small REST API.
5. Added a **SvelteKit web UI** (`software/frontend/splitweb`) and a filename-shortening build script.
6. Routed log messages to the third LCD message line.

Overall the structure is sensible: HTTP runs on core 0 and talks to the motor task only through its queue API,
and the UI has a dev-mode mock path. The problems are mostly in the edges: ISR safety, error paths, and where
layout knowledge lives.

---

## High

### H1. Reset button runs blocking, allocating code inside an interrupt
`fw/esp32/splitflap/main.cpp:59-75, 105-109`

`reset_button.enableInterrupt(buttonISR)` attaches `buttonISR` as a GPIO ISR, and `buttonISR` calls
`reset_button.read()`. In EasyButton 2.0.3, `read()` invokes the `onPressed` callback **synchronously**
(`EasyButton.cpp:55-62`), so `handle_reset()` runs in interrupt context. It then calls:
- `serialTask.log()`, which does `new std::string`, takes `DisplayTask`'s mutex with `portMAX_DELAY`, and calls
  `xQueueSendToBack`.
- `splitflapTask.resetAll()`, which calls `xQueueSendToBack(..., portMAX_DELAY)` wrapped in `assert`.

None of those are ISR-safe: heap allocation, a blocking mutex take, and blocking queue sends from interrupt
context. The ISR is also not `IRAM_ATTR`. The exact failure depends on timing. ESP-IDF commonly aborts on
blocking/locking calls from an ISR; otherwise the mutex can end up "owned" by whichever task was interrupted.

**Likely symptom:** an occasional or consistent reboot when the button is pressed (I haven't tested this on
hardware). Because every module re-homes on boot, a crash can *look* like a working reset.

**Fix:** don't do work in the ISR. Either poll the button from an existing task (for example call
`reset_button.read()` in `HTTPServerTask`'s or `DisplayTask`'s loop and drop `enableInterrupt`), or have the
ISR only set a flag / `xTaskNotifyFromISR` and do the reset in task context.

### H2. Web UI "Reset" button writes the word "reset" on the display
`web/src/routes/+page.svelte:51-55`

```ts
const doReset = async () => { setText('reset').then(...) }
```
This POSTs `/set` with `message=reset` instead of calling the existing `reset()` in `split_api.ts`, which POSTs
`/reset`. The deployed bundle has the same bug (`fw/data/web/_/i/n/2.1f9381ed.js`). The `/reset` endpoint is never
used by the UI. **Fix:** `import { reset }` and call it, then refresh status.

### H3. `Network::get_ssid()` returns a dangling pointer
`fw/esp32/splitflap/network.h:191-202`

`wifi_config_t conf` is a stack local, and the function returns `conf.sta.ssid`. Every caller
(`print_status()`, the DEBUG log in `_await_connection`) reads freed stack. That's undefined behaviour: it
usually "works", but it can print garbage or crash. **Fix:** return `WiFi.SSID()` (an Arduino `String`), or copy
into a member buffer.

### H4. Missing or blank filesystem leads to a reboot loop
`fw/esp32/splitflap/http_server_task.cpp:113-117`

If `SPIFFS.begin()` fails, `run()` does `return`. Returning from a FreeRTOS task function aborts on ESP-IDF, so
the device reboots and repeats. This happens on any board where the firmware was flashed but `uploadfs` wasn't
run, or after a partition-table change. **Fix:** log, skip `serveStatic`, still serve the API, and keep the task
alive (`vTaskDelete(NULL)` is the right way to end a task anyway). Consider `SPIFFS.begin(true)`
(format-on-fail).

### H5. WiFi "connected" flag is never cleared, so the reconnect logic is dead
`fw/esp32/splitflap/network.h:136-189`, `http_server_task.cpp:198-205`

`m_fWifiConnected` is only updated inside `_await_connection()`. After a drop it stays `true`.
`is_wifi_connected()` keeps returning true, `reconnect_wifi()` returns early, and `print_status()` never shows
"reconnecting". Reconnection currently relies entirely on the Arduino core's auto-reconnect. **Fix:** derive the
state from `WiFi.status() == WL_CONNECTED` (or a `WiFi.onEvent` handler) instead of caching it.

---

## Medium

### M1. Physical layout is handled only in the HTTP handler
`fw/esp32/splitflap/http_server_task.cpp:144-157`, `fw/esp32/splitflap/display_layouts.h:21,77`

- `/set` reverses the first `NUM_MODULES/2` characters to match the wiring (top row runs right-to-left).
  Nothing else does: text sent over the serial plaintext/protobuf protocols (or MQTT if re-enabled) comes out
  mirrored on the top row.
- The LCD preview still uses upstream's defaults, `DISPLAY_COLUMNS 6` with `getLayoutPositionSingleRowZigZag`.
  That draws 4 rows × 6 in a different order than the real 2 × 12 board.
- The 2-row assumption is hard-coded (`NUM_MODULES / 2`).

**Fix:** implement one layout function for the real wiring (row 0: `col = 11 - i`, row 1: `col = i - 12`) and set
`DISPLAY_COLUMNS 12`. Apply the logical→physical mapping in one place used by every input path (a wrapper around
`showString`).

### M2. `/text` reports a cache, not the display
`fw/esp32/splitflap/http_server_task.cpp:125-128, 159`

`last_text_` is updated only by HTTP `/set`. It's empty after boot and stale after any serial command, so the UI
then shows the wrong current text. **Fix:** build the string from `splitflap_task_.getState()`
(`flaps[modules[i].flap_index]`), applying the inverse layout mapping. Optionally include the target vs. current
text.

### M3. Toolchain and libraries are unpinned
`fw/platformio.ini:25, 39` (`platform = espressif32`, `bodmer/TFT_eSPI`), plus caret ranges for ESPAsyncWebServer
and EasyButton.

Rebuilding today pulls espressif32 7.1.3 / Arduino-ESP32 2.0.17 / TFT_eSPI 2.5.43. It does build, but it is
almost certainly not what's flashed on the device, and a future platform bump (Arduino-ESP32 3.x has breaking
API changes) can silently break things. Upstream also hit "serial issues" on a newer platform and reverted
(`05a0898`). **Fix:** pin exact versions. Smoke-test the current resolution on the real hardware first, then
record it.

### M4. Debug output written straight to the protocol port
`fw/esp32/splitflap/network.h:145`

`IF_DEBUG(WiFi.printDiag(Serial))` writes to the same USB-CDC stream the SerialTask uses for the
legacy/protobuf protocols. When the host has switched to protobuf framing this corrupts packets. **Fix:** route
through the logger or remove it.

### M5. HTTP handlers can block the AsyncTCP task
`http_server_task.cpp:157, 187` → `splitflap_task.cpp:331, 340`

`showString`/`resetAll` do `xQueueSendToBack(queue_, …, portMAX_DELAY)` on a 5-deep queue from inside the
AsyncTCP callback. Rapid-fire requests while the motor task is busy will stall all networking, and can trip the
async_tcp task watchdog. **Fix:** use a short timeout and return HTTP 503 when the queue is full.

### M6. Web UI has no error handling or live status
`web/src/routes/+page.svelte`, `web/src/lib/split_api.ts`

- Every `.then()` lacks a `.catch()`, so failures are unhandled promise rejections and the user gets no
  feedback. If `/text` fails on load, the board keeps displaying `loading...` as flap text.
- `postFlapString` sets `flapString = flapStringPending` and ignores the sanitized text the server returns.
  Characters the server replaced are then shown wrongly.
- Status is fetched once on mount and only refreshed manually. Module errors (the main reason for the status
  overlay) won't appear without clicking "Refresh Status". Poll every few seconds, or use websocket push.
- `<input>` has no `maxlength={NUM_MODULES}`. Extra typed characters move the cursor off the board and are then
  silently truncated by the server.
- Ctrl+key "fill" doesn't `preventDefault`. Ctrl+R / Ctrl+W fill the board and also reload or close the tab.
- Board dimensions are hard-coded (`constants.ts`) even though `/status` already returns the module count
  (there's a TODO in `FlapDisplay.svelte`).

---

## Low

| # | Where | Issue |
|---|---|---|
| L1 | `fw/esp32/splitflap/main.cpp:89-96` | `#if MQTT … mqttTask.begin()` and `#if HTTP … httpTask.begin()` remain while the object declarations are commented out, so enabling either flag fails to compile. |
| L2 | `fw/esp32/splitflap/main.cpp:33`, `serial_task.*` | `SerialTask` now requires a `DisplayTask&`, so `ENABLE_DISPLAY=false` no longer compiles. |
| L3 | `fw/esp32/splitflap/serial_task.cpp:34`, `main.cpp:77` | `Serial.begin()` is called twice. The 2 s `delay` in `setup()` is a debugging leftover. |
| L4 | `fw/esp32/splitflap/serial_task.cpp:88` | Every log line overwrites LCD message line 2, so useful status (IP address) gets replaced by chatter. Show IP/SSID on a dedicated line. |
| L5 | `fw/esp32/splitflap/network.h` | `HOSTNAME` from `secrets.h` is unused (hostname is hard-coded `"splitflap"` + MAC stub). No `MDNS.begin()`, although the comment mentions mDNS. Unused: `ArduinoLog` dependency, `esp_sntp.h`, NTP constants (plus a duplicate `#define NTP_POLLING_INTERVAL_MS`), `m_strMacStub`. |
| L6 | `fw/esp32/splitflap/http_server_task.cpp:74-101` | `sanitize_string` works on bytes, so a UTF-8 character (e.g. "é", "’" from a phone keyboard) becomes 2–3 spaces and shifts the rest of the line. |
| L7 | `fw/platformio.ini:52` | `-std=c++11` in `build_flags` also goes to C files (warning) and is redundant with the framework default. Use `build_unflags`/`build_cxx_flags` if needed. |
| L8 | `fw/esp32/core/splitflap_task.cpp:31` | Splitflap task stack raised 2 KB → 16 KB (upstream settled on 4 KB). HTTP_Server task has 32 KB for an idle 10 s loop. Not harmful at 21% RAM, but check high-water marks (`uxTaskGetStackHighWaterMark`) and right-size. |
| L9 | `fw/Splitflap/src/spi_io_config.h:25-33` | Pins aliased through Arduino SPI pin names (`LATCH_PIN MISO` = GPIO13, `PIN_NUM_MISO SS` = GPIO10). Use literal GPIO numbers. Stale comments: "HSPI to avoid ST7789 VSPI conflict", "3 MHz for classic driver". `config.h` pinout comment lists classic ESP32 pins. |
| L10 | `fw/Splitflap/Splitflap.ino` | Replaced by a "hello world" stub that the build excludes. Delete it, and `esp32/core/uart_stream.*` if it stays unused. |
| L11 | `.github/workflows/pio.yml` | CI builds envs that were deleted (`uno-*`, `esp32`, `chainlinkBase`, `chainlinkDriverTester`) and would also need a `secrets.h`. It fails on every push. |
| L12 | `fw/.gitignore:6` | Typo `secets.h`. Harmless, since `fw/esp32/.gitignore` already ignores `secrets.h`, but remove it. |
| L13 | `web/.gitignore` | `build_shortened/` isn't ignored (it was committed once, then deleted in `c2fa9f5`). No script copies the build into `fw/data/web/`, so that step is manual and error-prone. |
| L14 | `web/scripts/rename.js` | Exists only for SPIFFS' 32-char path limit. String-replacing paths inside JS/CSS is fragile (any path segment substring could match). This goes away with LittleFS/FFat. |
| L15 | `fw/data/web/` | Built artifacts are committed. Reasonable for "what's on the device", but they can drift from source. Either generate them in CI or note the source commit in `_/version.json`. |
| L16 | `electronics/chainlinkDriver/*`, `electronics/chainlinkBuddyTDisplay/*` | KiCad-7 `sym-lib-table` rewrites and `*-rescue.kicad_sym` files from opening the projects in a newer KiCad, with no intentional design change. Revert to reduce upstream merge noise. |
| L17 | `fw/package-lock.json`, `fw/.vscode/*`, `arduino/.vscode/*` | Stray empty npm lockfile and editor settings (cSpell word lists). |
| L18 | `web/package.json` | `"license": "MIT"` inside an Apache-2.0 repo. Fine if intentional; otherwise align. Unused `@sveltejs/adapter-auto` dep. Template `README.md`. Placeholder Playwright test. Flap font `'Roboto'` isn't bundled, so it falls back to a system sans-serif. |
| L19 | `fw/esp32/splitflap/http_server_task.cpp` | No auth on `POST /set` and `/reset`, and no CORS restriction. Any LAN device or web page could trigger them via a form POST. Acceptable at home; note it if the network is shared. |
| L20 | `fw/esp32/splitflap/serial_task.cpp` | Uses USB-CDC `Serial` (HWCDC). Check behaviour when no host is attached: older Arduino-ESP32 2.0.x HWCDC writes could stall per write when no host was reading, which would slow the log drain. |

---

## Suggested fix order

1. H1 (button), H2 (UI reset), H3 (dangling SSID), H4 (FS reboot loop): each is a small, local change.
2. M3: pin the versions you validate on hardware while doing step 1.
3. M1 + M2 together: introduce the layout mapping, then derive `/text` from state.
4. H5, M4, M5, and the M6 UI items.
5. Low items as housekeeping, ideally in the same pass as any upstream merge (see 03-upstream-review.md), since
   several (L10, L11, L16) affect merge conflicts.
