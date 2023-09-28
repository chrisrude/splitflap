<script lang="ts">
    import { DEFAULT_FLAP_VALUE, MODULES_PER_ROW, NUM_MODULES } from '../constants';
    import Flap from './Flap.svelte';

    const NUM_ROWS = NUM_MODULES / MODULES_PER_ROW;

    export let flapString: string;

    let flapValues: string[];
    // set flapValues from flapString.
    // if flapString is longer than NUM_MODULES, truncate it.
    // if flapString is shorter than NUM_MODULES, pad it with DEFAULT_FLAP_VALUE
    // to make it NUM_MODULES long.
    if (flapString.length > NUM_MODULES) {
        flapValues = flapString.slice(0, NUM_MODULES).split('');
    } else {
        flapValues = flapString.padEnd(NUM_MODULES, DEFAULT_FLAP_VALUE).split('');
    }
</script>

<div class="flapDisplay">
    <div style="display: contents; --modules-per-row: {MODULES_PER_ROW}">
        <!-- display a Flap for each value in flapValues, listing only 
             MODULES_PER_ROW in each flapRow-->
        {#each { length: NUM_ROWS } as _, i}
            <div class="flapRow">
                {#each flapValues.slice(i * MODULES_PER_ROW, (i + 1) * MODULES_PER_ROW) as value, j}
                    <Flap {value} />
                {/each}
            </div>
        {/each}
    </div>
</div>

<!-- style flapDisplay using flex to show each flapRow in a line which uses 
the whole width of the window -->
<style>
    .flapDisplay {
        display: flex;
        flex-direction: column;
        justify-content: center;
        align-items: center;
        width: 100%;
    }

    .flapRow {
        display: flex;
        justify-content: center;
        align-items: center;
        width: 100%;
    }
</style>
