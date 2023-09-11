#pragma once

#include <Arduino.h>

/***** CONFIGURATION *****/

// 1) Mode
#define SENSOR_TEST false

// 2) General Settings
#ifndef NUM_MODULES
#define NUM_MODULES (1)
#endif

// Whether to force a full rotation when the same letter is specified again
#define FORCE_FULL_ROTATION true

// Whether to use/expect a home sensor. Enable for auto-calibration via home
// sensor feedback. Disable for basic open-loop control (useful when first
// testing the split-flap, since home calibration can be tricky to fine tune)
#define HOME_CALIBRATION_ENABLED true

// 3) Flap Contents & Order
#define NUM_FLAPS (40)

// This should match the order of flaps on the spool, with the first being the
// "home" flap.
const uint8_t flaps[NUM_FLAPS] = {
  ' ',
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
  '.',
  ',',
  '\'',
};

// 5) Board-dependent Default Settings
#define MONITOR_SPEED 230400

/*************************/


/**
 * Suggested Pinouts
 *
 * ESP32 - SPI IO (Shift Register) - Up to 120+ modules:
 *     25 Latch         (out)
 *     33 CLK           (out)
 *     39 MISO          (in)
 *     32 MOSI          (out)
 *
 */


// Configuration validation and dependent settings
#if NUM_MODULES < 1
#error NUM_MODULES must be at least 1
#endif
#ifdef CHAINLINK
#define NUM_LOOPBACKS (NUM_MODULES / 3)
#define CHAINLINK_ENFORCE_LOOPBACKS 1
#endif
