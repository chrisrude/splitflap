/***
 * This file contains the functions that call the split flap API
 */

import { getDefaultStatus, type FlapStatus } from "./flap_status";

import { dev } from '$app/environment';
import { FLAP_CHARACTERS } from "./constants";

/**
 * @returns Returns the character set that the flaps support.
 */
export const getFlaps = async (): Promise<string[]> => {
    const res = await fetch('/flaps');
    if (!res.ok) {
        if (dev) {
            // in dev mode, pretend that the server returned the
            // default character set
            return FLAP_CHARACTERS;
        }
        console.error(res);
        throw new Error('Failed to get flap values');
    }
    const flapValues = await res.text();
    return flapValues.split('');
};

let lastGetFailed = false;

/**
 * @returns Returns the text on the flaps.
 */
export const getText = async (): Promise<string> => {
    const res = await fetch('/text');
    if (!res.ok) {
        if (dev) {
            // in dev mode, pretend that the server returned this
            // placeholder text
            return 'Hello World';
        }
        if (!lastGetFailed) {
            // do this to not spam the console
            console.error('Failed to get flap string');
            console.error(res);
        }
        lastGetFailed = true;
        throw new Error('Failed to get flap string');
    }
    const flapString = await res.text();
    lastGetFailed = false;
    return flapString;
};

/**
 * Get the status of the flaps.  This indirectly returns the
 * number of flaps on the display.
 * @returns Returns the status of the flaps.  See FlapStatus for
 *    more information.
 */
export const getStatus = async (): Promise<FlapStatus[]> => {
    const res = await fetch('/status');
    if (!res.ok) {
        if (dev) {
            // in dev mode, return a hard-coded status object
            // which shows a variety of different states    
            const flapStatusValues = getDefaultStatus();
            flapStatusValues[flapStatusValues.length - 1].count_missed_home = 3;
            flapStatusValues[flapStatusValues.length - 2].count_unexpected_home = 2;
            flapStatusValues[flapStatusValues.length - 3].state = 'sensor_error';
            return flapStatusValues
        }

        console.error(res);
        throw new Error('Failed to get flap status');
    }
    const status = await res.json();
    return status;
};

/**
 * Sets the text on the flaps.
 * 
 * @param message Message to show on the flaps.  Should only contain
 *     characters in the character set (returned by getFlaps).  Must
 *     be less than the max length (returned by getStatus).
 * @returns   Returns the new text.
 * The difference is that the server will filter out invalid
 * characters, so it might not show exactly what you sent.
 */
export const setText = async (message: string): Promise<string> => {

    const formData = new FormData();
    formData.append('message', message);

    const res = fetch('/set', {
        method: 'POST',
        body: formData
    });

    const result = await res;
    if (!result.ok) {
        if (dev) {
            // in dev mode, pretend that the server returned the
            // text we asked for
            return message;
        }
        console.error(result);
        throw new Error('Failed to set flap string');
    }
    const new_text = await result.text();
    return new_text;
};


/***
 * Resets the flaps to the default state.  Re-homes the sensors
 * and sets the text to a blank space.
 */
export const reset = async (): Promise<void> => {
    // post to the reset endpoint
    const res = await fetch('/reset', {
        method: 'POST'
    });
    if (!res.ok) {
        if (dev) {
            // in dev mode, pretend that the server reset the flaps
            return;
        }
        console.error(res);
        throw new Error('Failed to reset flaps');
    }
}