<script lang="ts">
    import { dev } from '$app/environment';
    import FlapDisplay from '$lib/components/FlapDisplay.svelte';
    import { FLAP_CHARACTERS, MODULES_PER_ROW, NUM_MODULES } from '$lib/constants';
    import { createDefaultFlapStatus, type FlapStatus } from '$lib/flap_status';
    import { getFlaps, getStatus, getText, setText } from '$lib/split_api';
    import { onMount } from 'svelte';

    let flapString: string = 'loading...';
    let flapStringPending: string = '';

    let flapStatusValues: FlapStatus[] = Array(NUM_MODULES);
    for (let i = 0; i < NUM_MODULES; i++) {
        flapStatusValues[i] = createDefaultFlapStatus();
    }
    let allowedFlapValues = FLAP_CHARACTERS;

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

    const getFlapStatus = async () => {
        getStatus().then((status) => {
            flapStatusValues = status;
        });
    };

    const getFlapString = async () => {
        getText().then((text) => {
            flapString = text;
        });
    };

    const postFlapString = async () => {
        setText(flapStringPending).then(() => {
            flapString = flapStringPending;
            flapStringPending = '';
        });
    };

    // test data for FlapDisplay
    if (dev) {
        flapStatusValues[flapStatusValues.length - 1].count_missed_home = 3;
        flapStatusValues[flapStatusValues.length - 2].count_unexpected_home = 2;
        flapStatusValues[flapStatusValues.length - 3].state = 'sensor_error';
    }

    let ref: HTMLInputElement;

    onMount(() => {
        ref.focus();
        getFlaps().then((flaps) => {
            allowedFlapValues = flaps;
        });
        getFlapString();
        getFlapStatus();
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
<button on:click={getFlapStatus} class="button-89">Refresh Status</button>

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
