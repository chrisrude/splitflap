<script lang="ts">
    import type { FlapStatus } from '$lib/flap_status';
    import { DEFAULT_FLAP_VALUE, MODULES_PER_ROW, NUM_MODULES } from '../constants';
    import Flap from './Flap.svelte';

    // todo: dynamically adjust based on return values from /status
    const NUM_ROWS = NUM_MODULES / MODULES_PER_ROW;

    type FlapLetterStatus = {
        current: string;
        pending?: string;
    };

    export let flapString: string;
    export let flapStringPending: string | undefined;
    export let flapStatusValues: FlapStatus[];

    let flapValues: FlapLetterStatus[] = Array(NUM_MODULES).fill({ current: DEFAULT_FLAP_VALUE });

    const updateValues = () => {
        // loop through each FlapStatus in flapValues and set its current and pending values
        // to the corresponding values in currentValues and pendingValues
        flapValues = flapValues.map((_, i) => {
            return {
                current: flapString[i] ?? DEFAULT_FLAP_VALUE,
                pending: flapStringPending?.[i] ?? undefined
            };
        });
    };

    $: (undefined !== flapString || undefined !== flapStringPending) && updateValues();
</script>

<div class="flapDisplay">
    <div style="display: contents; --modules-per-row: {MODULES_PER_ROW}">
        <!-- display a Flap for each value in flapValues, listing only 
             MODULES_PER_ROW in each flapRow-->
        {#each { length: NUM_ROWS } as _, i}
            <div class="flapRow">
                {#each flapValues.slice(i * MODULES_PER_ROW, (i + 1) * MODULES_PER_ROW) as value, j}
                    <Flap
                        value={value.current}
                        pendingValue={value.pending}
                        status={flapStatusValues[i * MODULES_PER_ROW + j]}
                        selected={i * MODULES_PER_ROW + j === flapStringPending?.length}
                    />
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
