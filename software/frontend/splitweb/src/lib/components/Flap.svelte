<script lang="ts">
    export let value: string;
</script>

<div class="flap">
    {#if value === ' '}
        &nbsp;
    {:else}
        {value}
    {/if}
</div>

<!-- style .flap to be a large white character in a black box, with rounded corners
and a white line through the middle.  Use a flex display -->
<style>
    .flap {
        --flap-width: calc(100% / var(--modules-per-row));
        --split-height: 2px;

        display: flex;
        justify-content: center;
        align-items: center;
        width: var(--flap-width);
        height: calc((var(--flap-width) * 86 / 54) + var(--split-height));
        background-color: black;
        color: white;
        font-size: 120px;
        border-radius: 10px;
        border: 2px solid white;
        font-family: 'Roboto', sans-serif;
        font-weight: 500;
    }

    /* style the line through the middle of the flap */
    .flap::before {
        content: '';
        position: absolute;
        width: 100%;
        height: var(--split-height);
        background-color: white;
    }
</style>
