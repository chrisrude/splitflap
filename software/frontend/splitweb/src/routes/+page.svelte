<script lang="ts">
    import FlapDisplay from '$lib/components/FlapDisplay.svelte';
    import { FLAP_CHARACTERS, MODULES_PER_ROW, NUM_MODULES } from '$lib/constants';
    import { createDefaultFlapStatus, type FlapStatus } from '$lib/flap_status';
    import { onMount } from 'svelte';
    let flapString: string = 'Hello World!';
    let flapStringPending: string = '';

    let lastGetFailed: boolean = false;
    let flapStatusValues: FlapStatus[] = Array(NUM_MODULES);
    for (let i = 0; i < NUM_MODULES; i++) {
        flapStatusValues[i] = createDefaultFlapStatus();
    }
    let allowedFlapValues = FLAP_CHARACTERS;

    const getFlapValues = async () => {
        // call /flaps
        const res = await fetch('/flaps');
        if (res.ok) {
            const flapValues = await res.text();
            allowedFlapValues = flapValues.split('');
            console.log(flapValues);
        } else {
            console.error('Failed to get flap values');
            console.error(res);
        }
    };

    const getFlapString = async () => {
        const res = await fetch('/text');
        if (res.ok) {
            flapString = await res.text();
            lastGetFailed = false;
        } else {
            if (!lastGetFailed) {
                // do this to not spam the console
                console.error('Failed to get flap string');
                console.error(res);
            }
            lastGetFailed = true;
        }
    };

    const getStatus = async () => {
        const res = await fetch('/status');
        if (res.ok) {
            const status = await res.json();
            console.log(status);
            flapStatusValues = status;
        } else {
            console.error('Failed to get flap status');
            console.error(res);
        }
    };

    const postFlapString = async () => {
        const formData = new FormData();
        formData.append('message', flapStringPending);

        const res = fetch('/set', {
            method: 'POST',
            body: formData
        });
        // when we get a response, log it to the console
        const result = await res;
        if (result.ok) {
            const set_text = await result.text();
            flapString = set_text;
            flapStringPending = '';
        } else {
            console.error(result);
        }
        return res;
    };

    const addSpaces = () => {
        // if we're not on the last row of input, add spaces to postFlapString
        // until we can reach the next line
        const line: number = flapStringPending.length / MODULES_PER_ROW;
        const num_rows = NUM_MODULES / MODULES_PER_ROW;
        if (line < num_rows - 1) {
            const spaces = ' '.repeat(
                MODULES_PER_ROW - (flapStringPending.length % MODULES_PER_ROW)
            );
            flapStringPending += spaces;
        }
    };

    // fill entire display with a character
    const fillAll = (char: string) => {
        flapStringPending = char.repeat(NUM_MODULES);
    };

    flapStatusValues[flapStatusValues.length - 1].count_missed_home = 3;
    flapStatusValues[flapStatusValues.length - 2].count_unexpected_home = 2;
    flapStatusValues[flapStatusValues.length - 3].state = 'sensor_error';

    let ref: HTMLInputElement;

    onMount(() => {
        ref.focus();
        getFlapString();
        getStatus();
        getFlapValues();
    });
</script>

<FlapDisplay bind:flapString bind:flapStringPending bind:flapStatusValues />

<!-- post to /set endpoint in background -->
<form on:submit|preventDefault={postFlapString} class="inputForm">
    <label>
        Display:
        <input
            bind:value={flapStringPending}
            bind:this={ref}
            on:keydown={(e) => {
                if ((e.key === 'Enter' && e.shiftKey === true) || e.key === 'Tab') {
                    addSpaces();
                    e.preventDefault();
                } else if (e.ctrlKey && e.key.length === 1) {
                    fillAll(e.key);
                } else if (e.key.length === 1 && !allowedFlapValues.includes(e.key.toLowerCase())) {
                    // ignore keys that are not in allowedFlapValues
                    e.preventDefault();
                }
            }}
        />
    </label>
</form>

<!-- button which will call getFlapString -->
<button on:click={getFlapString} class="button-89">Refresh</button>
<button on:click={getStatus} class="button-89">Refresh Status</button>

<svelte:window
    on:click={() => {
        ref.focus();
    }}
/>

<style>
    .inputForm {
        width: 0;
        overflow: hidden;
    }
    .button-89 {
        --b: 3px; /* border thickness */
        --s: 0.45em; /* size of the corner */
        --color: #373b44;

        padding: calc(0.5em + var(--s)) calc(0.9em + var(--s));
        color: var(--color);
        --_p: var(--s);
        background: conic-gradient(from 90deg at var(--b) var(--b), #0000 90deg, var(--color) 0)
            var(--_p) var(--_p) / calc(100% - var(--b) - 2 * var(--_p))
            calc(100% - var(--b) - 2 * var(--_p));
        transition: 0.3s linear, color 0s, background-color 0s;
        outline: var(--b) solid #0000;
        outline-offset: 0.6em;
        font-size: 16px;

        border: 0;

        user-select: none;
        -webkit-user-select: none;
        touch-action: manipulation;
    }

    .button-89:hover,
    .button-89:focus-visible {
        --_p: 0px;
        outline-color: var(--color);
        outline-offset: 0.05em;
    }

    .button-89:active {
        background: var(--color);
        color: #fff;
    }
</style>
