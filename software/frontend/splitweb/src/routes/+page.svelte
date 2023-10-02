<script lang="ts">
    import FlapDisplay from '$lib/components/FlapDisplay.svelte';
    import { MODULES_PER_ROW, NUM_MODULES } from '$lib/constants';
    import { onMount } from 'svelte';
    let flapString: string = 'Hello World!';
    let flapStringPending: string = '';

    // todo: periodically call /get and update flapString

    const postFlapString = async () => {
        const res = fetch('/set', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ flapStringPending })
        });
        // when we get a response, log it to the console
        const result = await res;
        if (result.ok) {
            // todo: get flapString from result
            flapString = flapStringPending;
            flapStringPending = '';
        } else {
            // todo: handle error
            flapString = flapStringPending;
            flapStringPending = '';
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
    let ref: HTMLInputElement;

    onMount(() => {
        ref.focus();
    });
</script>

<FlapDisplay bind:flapString bind:flapStringPending />

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
                }
            }}
        />
    </label>

    <button>Set</button>
</form>

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
</style>
