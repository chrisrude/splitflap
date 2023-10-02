<script lang="ts">
    import FlapDisplay from '$lib/components/FlapDisplay.svelte';
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
</script>

<FlapDisplay bind:flapString bind:flapStringPending />

<!-- post to /set endpoint in background -->
<form on:submit|preventDefault={postFlapString}>
    <label>
        Display:
        <input bind:value={flapStringPending} id="nextFlap" />
    </label>

    <button>Set</button>
</form>
