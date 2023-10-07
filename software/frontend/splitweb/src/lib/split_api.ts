/***
 * This file contains the functions that call the split flap API
 */

import type { FlapStatus } from "./flap_status";


/**
 * @returns Returns the character set that the flaps support.
 */
export const getFlaps = async (): Promise<string[]> => {
    const res = await fetch('/flaps');
    if (!res.ok) {
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
        console.error(result);
        throw new Error('Failed to set flap string');
    }
    const new_text = await result.text();
    return new_text;
};
