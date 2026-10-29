# Upstream changes since the fork: take, cherry-pick, or skip?

_Compared `8c3520d...scottbez1/splitflap@master` via the GitHub API on 2026-10-29: **152 commits, 285 files**,
none of them in this fork. Latest upstream commit: `87b17c5` (2026-08-14, docs). Latest firmware change:
`fb789e0` (2025-08-10, MQTT + OTA)._

## Recommendation: **take in part**. Don't merge wholesale right now.

Cherry-pick or hand-port a small, specific set of firmware changes (listed below), and skip the hardware/docs
churn. Treat "rebase the fork onto upstream" as a separate, optional project for later, if you decide you want
to track upstream long-term.

Why not merge everything:

1. **Upstream firmware still targets the classic ESP32**: `board = esp32dev`, `platform = espressif32@3.4`
   (Arduino-ESP32 1.0.x era), TFT_eSPI 2.4.25 with SPI ST7789 pins. Upstream tried moving to espressif32 6.10
   in `62c315a` and **reverted it** in `05a0898` because of serial issues. Your S3 port depends on a modern
   platform, so the `platformio.ini` and pin config would have to be redone after any merge.
2. **Repo restructure**: `arduino/splitflap/**` moved to `firmware/**`, and `platformio.ini` moved to the repo
   root (`0811d99`, #245). Every firmware file this fork touched was also renamed upstream. `Splitflap.ino` was
   deleted upstream but modified here (a modify/delete conflict). New local files (`http_server_task.*`,
   `network.h`, `data/web`) would need to be moved by hand.
3. **Default character set changed** to the v2 52-flap set, with the convention that **letters are UPPERCASE and
   lowercase `g/p/r/w/y` mean color blocks** (`firmware/src/config.h`). Your hardware has 40 lowercase-mapped
   flaps, and the HTTP code lowercases input. Taken blindly, this would break the physical mapping. Upstream
   tools (the web-serial demo) would also render your `g p r w y` letters as color blocks.
4. **Most of the 285 files are hardware and docs for the v2 mechanical design**: a new 52-flap enclosure, a new
   SMD sensor PCB, connector changes, and 3D-printable flaps. These are not useful for an already-built display,
   and upstream says the new sensors are **not compatible with v0.7-and-older laser-cut hardware**.
5. Upstream activity is now low (docs fixes in 2025-26), so missing future upstream changes costs little.

## What changed upstream, by area

| Area | Key commits | Relevance to this build |
|---|---|---|
| **Firmware: software offset calibration + persistence** | `55ec804`, `9473d44`, `b50cc17`, `f097f36`, `3f06e88` (in the v2 merge `fec007d`) | **High value.** Per-module home offset adjustable in 1/10 or 1/2 flap steps, "set current position as home", and saving all offsets to FFat (`configuration.cpp/.h`, a `PersistentConfiguration` proto). It also simplifies `splitflap_module.h` (removes the gear-ratio fraction math, `STEPS_PER_REVOLUTION 2048`; same value for the 28BYJ-48 as before). Lets you fix misaligned modules without opening the case. |
| Firmware: protobuf protocol additions | `a63568d`, `1265147`, `f74511c`, `edee41a` | `GeneralState` (build info, uptime, flap character set), `loopbacks_ok`, calibration actions, `save_all_offsets`. **Useful mainly because it lets upstream's hosted Web Serial tool calibrate your display over USB with no UI work on your side.** |
| Firmware: `showString` overflow fix | `8db0a90` (#224) | **Take.** One line: `i < length && i < NUM_MODULES`. Your current callers already bound the length, so this is defensive. |
| Firmware: `showString(..., default_unspecified_home)` | in `fec007d` | Nice-to-have: modules past the end of a short string go blank. Your HTTP path already pads. |
| Firmware: MQTT + Home Assistant discovery + ArduinoOTA | `938dfbc` (#240), `c286b24` (#247), `fb789e0` (#266) | **Worth porting selectively.** OTA is very useful for an assembled display and only needs a few lines. MQTT/HA discovery is useful if you run Home Assistant. Their task polls `WiFi.status()` properly, a pattern your `network.h` should copy (see review H5). |
| Firmware: WiFi sleep disabled to avoid SPI glitches | `62c315a` (#263) | **Probably not needed.** The root cause is ESP32 errata 3.11 on GPIO36/39 (classic ESP32 only); your S3 reads MISO on GPIO10. Only add `WiFi.setSleep(false)` if you see spurious "unexpected home" counts with WiFi on. |
| Firmware: build-info macros, debug logging | `f74511c`, `firmware/buildscript_build_info_macros.py` | Nice-to-have: the git hash at boot and in the API helps tell which firmware is on the wall. |
| Firmware: `NUM_MODULES % 6 == 0` check for Chainlink | in `fec007d` | Harmless; 24 passes. |
| Firmware: platform/library pins | `01bebd8`, `05a0898` | **Skip the versions** (classic-ESP32 platform), but **copy the practice** of pinning everything. |
| Firmware: Arduino IDE / AVR removal, move to `firmware/` | `0811d99` (#245) | You already removed AVR support. Adopt the layout only if you do a full rebase. |
| JS: Web Serial rewrite, monorepo (`splitflapjs-core/-webserial/-node`), React demo, GitHub Pages | `9ea173f` (#220), `a65cfad`, `d00a0d8`, `bd32f5d`, `7f00bd3` | Not needed for your HTTP UI. The hosted page is still useful as a USB calibration/diagnostic tool if you port the proto changes. `splitflapjs-core` is a serial transport, so you can't reuse it for HTTP. |
| Python host library | `edee41a` (#242), `e1eb9b9` (#232) | Only if you drive the display from a PC over USB. Needs the proto changes. |
| 3D / mechanical v2 (52 flaps, spool/enclosure, connector) | `dd5c942`…`b272627`, `b7464e6`, `bf70d42`, `d542894`, `e5e045b` (#248), `1001f48` (#238) | **Skip** unless you build more modules. If you do, read upstream's v2 docs directly; v2 parts aren't compatible with older sensors/enclosures. |
| 3D-printable flaps | `d6452ae` (#256), `fa79c56` (#260) | Skip, unless you want to make replacement flaps. |
| Electronics: v2 SMD sensor, Chainlink Driver BOM/placement update, removal of classic boards | `31790bf`, `c9b0980`…`8b58d1c`, `fcb016e` (#251) | Skip for the existing display. If you order more Chainlink Drivers, use upstream's current BOM/CPL (`fcb016e`). |
| Docs migrated into repo (assembly, ordering, electronics guide, flap guide) | `9ae6a3a`…`4b056c5`, `e7ace5c`, `87b17c5` | Read on GitHub; no need to merge. `docs/ElectronicsGuide.md` has useful troubleshooting/calibration notes (`e7ace5c`). |
| CI (kibot v6, kikit, node action bumps) | various | Skip. Your CI needs its own fix anyway (review L11). |

## Concrete cherry-pick plan

In order of value per effort:

1. **`8db0a90` showString bounds.** Trivial; apply by hand (the file path differs).
2. **OTA.** Lift the `ArduinoOTA` block from upstream `firmware/esp32/splitflap/mqtt_task.cpp` (`fb789e0`) into
   `HTTPServerTask::run()` (`ArduinoOTA.begin()` after WiFi, then `ArduinoOTA.handle()` in the loop, which must
   then run every ~10 ms instead of every 10 s). Add `OTA_PASSWORD` to `secrets.h.example`, and add an
   `env:chainlink_ota` like upstream's.
3. **Calibration + persistence (hand-port).** Bring over, adapted to the old paths:
   - `proto/splitflap.proto` additions, then regenerate `esp32/proto_gen/*` with the nanopb submodule (the
     runtime is still `Nanopb @ 0.4.6`).
   - `esp32/core/configuration.{h,cpp}`.
   - The `splitflap_module.h` offset changes, and the `splitflap_task.{h,cpp}` `QCMD_INCR_OFFSET_*`,
     `QCMD_SET_OFFSET`, `SAVE/RESTORE_ALL_OFFSETS` changes.
   - The `main.cpp` load-and-restore at boot.
   - The `serial_proto_protocol.cpp` command handling.
   - **Filesystem decision first:** upstream stores the config on FFat with `default_ffat.csv`, while your web
     UI lives on SPIFFS. On the 16 MB S3, pick one filesystem for both (FFat or LittleFS) with a custom or
     `app3M_fat9M_16MB.csv`-style partition table, and update `HTTPServerTask` to serve from it. That also
     removes the need for `rename.js`. Changing the partition table erases the filesystem, so you'll need to
     re-upload the web UI afterwards.
   - Expose calibration over HTTP (e.g. `POST /module/<i>/offset?step=tenth|half`, `POST /module/<i>/home`,
     `POST /calibration/save`) and add a calibration view to the Svelte UI. Or use upstream's hosted Web
     Serial page over USB.
   - Upstream bug to avoid when porting: in `SAVE_ALL_OFFSETS`, the "module isn't idle" path does `return`
     from `processQueue()` instead of `break`.
4. **If you port `GeneralState` or want to use upstream tooling:** switch `flaps[]` to upstream's "Flap option 1"
   (same 40 flaps, **uppercase** letters). Update `sanitize_string` (`toLowerCase` → `toUpperCase`) and the
   web UI's `FLAP_CHARACTERS` to match. Do this as one atomic change, because the HTTP API, LCD, and UI all
   assume lowercase today.
5. **MQTT / Home Assistant** (optional): port `mqtt_task.cpp` from `fb789e0`. It needs `json11` (already in
   `lib/`) and `-DMQTT_MAX_PACKET_SIZE=512`. If both run, share one WiFi bring-up between the HTTP and MQTT
   tasks instead of each calling `WiFi.begin()`.

## If you later want a full rebase onto upstream

That's a reasonable project if you want to follow upstream long-term. Expect roughly:

- Merge `upstream/master` with the fork base `8c3520d` as merge-base. Then resolve: `platformio.ini`
  (rename + rewrite), `config.h`, `spi_io_config.h`, `main.cpp`, `serial_task.*`, `splitflap_task.cpp`,
  `uart_stream.cpp`, and the deleted `Splitflap.ino`.
- Move `http_server_task.*`, `network.h`, `lib/json11`, and `data/web` under `firmware/`. Add a new
  `[env:chainlink_s3]` instead of editing `esp32base`, so upstream's classic-ESP32 envs stay intact and future
  merges stay cleaner.
- Choose the 40-flap uppercase set (above). Reconcile filesystems (FFat).
- Revert the local KiCad churn (review L16) and drop the dead `.ino` first, to cut conflicts.

Doing the clean-ups in 02-fork-review.md first (especially L10, L11, L16) makes either path easier.
