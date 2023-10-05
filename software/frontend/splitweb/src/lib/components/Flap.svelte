<script lang="ts">
    import { isFlapOk, isFlapWarning, type FlapStatus } from '$lib/flap_status';
    import FlapStatusDisplay from './FlapStatusDisplay.svelte';

    export let value: string;
    export let pendingValue: string | undefined;
    export let selected: boolean = false;
    export let status: FlapStatus;
</script>

<div class="flap" class:selected class:pendingValue={pendingValue !== undefined}>
    {#if pendingValue !== undefined}
        {pendingValue}
    {:else}
        {value}
    {/if}
    <div class="flap_status" class:error={!isFlapOk(status)} class:warning={isFlapWarning(status)}>
        <FlapStatusDisplay {status} />
    </div>
</div>

<!-- style .flap to be a large white character in a black box, with rounded corners
and a white line through the middle.  Use a flex display -->
<style>
    .flap {
        --flap-width: calc(100vw / var(--modules-per-row));
        --flap-height: calc(var(--flap-width) * 86 / 54);
        --split-height: 1px;
        --select-color: rgb(180, 84, 29);
        --radius-size: calc(var(--flap-width) / 10);
        --border-width: 2px;
        --box-shadow-width: calc(var(--border-width) * 4);

        display: flex;
        justify-content: center;
        align-items: center;
        width: var(--flap-width);
        height: var(--flap-height);
        background-color: black;
        color: white;
        font-size: var(--flap-width);
        border-radius: var(--radius-size);
        border: var(--border-width) solid white;
        font-family: 'Roboto', sans-serif;
        font-weight: 500;
        text-transform: uppercase;
        position: relative;
    }

    /* style the line through the middle of the flap */
    .flap::before {
        content: '';
        position: absolute;
        width: 100%;
        height: var(--split-height);
        background-color: white;
    }

    /* .flap.pendingValue:hover,
    .flap:hover {
        border: solid var(--border-width) var(--select-color);
        box-shadow: inset 0px 0px var(--box-shadow-width) var(--box-shadow-width)
            var(--select-color);
    } */

    .flap.pendingValue {
        color: var(--select-color);
    }

    .flap.selected::after {
        position: absolute;
        content: '_';
        animation: cursor-blink 1s steps(1) infinite;
    }

    @keyframes cursor-blink {
        0% {
            color: var(--select-color);
        }
        50% {
            color: black;
        }
    }

    /* create a triangle in the upper right corner */
    .flap_status.warning::before,
    .flap_status.error::before {
        content: '';
        position: absolute;
        width: calc(var(--radius-size) / 2);
        height: calc(var(--radius-size) / 2);
        top: 0;
        right: 0;
        border-bottom: var(--radius-size) solid transparent;
        border-left: var(--radius-size) solid transparent;
        border-top: var(--radius-size) solid red;
        border-right: var(--radius-size) solid red;
    }

    .flap_status.warning::before {
        border-top: var(--radius-size) solid orange;
        border-right: var(--radius-size) solid orange;
    }

    .flap_status {
        position: absolute;
        top: 0;
        right: 0;
        font-size: 0.5rem;
        padding: calc(var(--radius-size) / 4);
        color: rgba(255, 255, 255, 0);
        border-radius: var(--radius-size);
        width: calc(var(--flap-width) - 1rem);
        padding-top: 1rem;
    }

    .flap_status.error,
    .flap_status.warning:hover {
        color: lightpink;
        background-color: rgba(0, 0, 0, 0.6);
    }
</style>
