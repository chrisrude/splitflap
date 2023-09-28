<script lang="ts">
    import FlapDisplay from '$lib/components/FlapDisplay.svelte';
    export let flapString: string = 'Hello World!';
</script>

<FlapDisplay {flapString} />
