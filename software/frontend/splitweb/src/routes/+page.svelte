<script lang="ts">
    import FlapDisplay from '$lib/components/FlapDisplay.svelte';
    let flapString: string = 'Hello World!';
</script>

<FlapDisplay {flapString} />

<input type="text" bind:value={flapString} />
