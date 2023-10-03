<script lang="ts">
    import { isFlapOk, type FlapStatus } from '$lib/flap_status';
    import FlapStatusDisplay from './FlapStatusDisplay.svelte';

    export let value: string;
    export let pendingValue: string | undefined;
    export let selected: boolean = false;
    export let status: FlapStatus;
</script>

<div class="flap">
    {#if pendingValue !== undefined}
        <div class="flapPendingValue">
            {pendingValue}
        </div>
    {:else}
        <div class="flapActualValue" class:selected>
            {value}
        </div>
    {/if}
    <div class="flap_status" class:error={!isFlapOk(status)}>
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

        display: flex;
        justify-content: center;
        align-items: center;
        width: var(--flap-width);
        height: var(--flap-height);
        background-color: black;
        color: white;
        font-size: var(--flap-width);
        border-radius: var(--radius-size);
        border: 2px solid white;
        font-family: 'Roboto', sans-serif;
        font-weight: 500;
        text-transform: uppercase;
        white-space: pre;
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

    .flapPendingValue {
        color: var(--select-color);
    }

    .selected {
        border-bottom: var(--radius-size) solid var(--select-color);
        border-radius: var(--radius-size);
        margin-bottom: -1rem;
    }

    /* create a triangle in the upper right corner */
    .flap_status.error::before {
        content: '';
        position: absolute;
        width: calc(var(--radius-size) / 2);
        height: calc(var(--radius-size) / 2);
        top: 0;
        right: 0;
        border-bottom: 0.5rem solid transparent;
        border-left: 0.5rem solid transparent;
        border-top: 0.5rem solid red;
        border-right: 0.5rem solid red;
    }

    .flap_status {
        position: absolute;
        top: 0;
        right: 0;
        font-size: 0.5rem;
        padding: calc(var(--radius-size) / 2);
        color: rgba(255, 255, 255, 0);
        border-radius: var(--radius-size);
    }

    .flap_status.error,
    .flap_status:hover {
        color: lightpink;
        background-color: rgba(0, 0, 0, 0.6);
    }
</style>
