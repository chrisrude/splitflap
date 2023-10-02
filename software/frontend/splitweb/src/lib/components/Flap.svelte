<script lang="ts">
    export let value: string;
    export let pendingValue: string | undefined;
</script>

<div class="flap">
    {#if pendingValue !== undefined}
        <div class="flapPendingValue">
            {pendingValue}
        </div>
    {:else}
        <div class="flapActualValue">
            {value}
        </div>
    {/if}
</div>

<!-- style .flap to be a large white character in a black box, with rounded corners
and a white line through the middle.  Use a flex display -->
<style>
    .flap {
        --flap-width: calc(100vw / var(--modules-per-row));
        --flap-height: calc(var(--flap-width) * 86 / 54);
        --split-height: 1px;

        display: flex;
        justify-content: center;
        align-items: center;
        width: var(--flap-width);
        height: var(--flap-height);
        background-color: black;
        color: white;
        font-size: var(--flap-width);
        border-radius: 10px;
        border: 2px solid white;
        font-family: 'Roboto', sans-serif;
        font-weight: 500;
        text-transform: uppercase;
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
        color: green;
    }
</style>
