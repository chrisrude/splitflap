<script lang="ts">
    import type { FlapStatus } from '$lib/flap_status';

    export let status: FlapStatus;
</script>

<div class="flap_status">
    <!--
    export type FlapState =
    | 'normal'
    | 'look_for_home'
    | 'sensor_error'
    | 'panic'
    | 'state_disabled'
    | 'unknown';
export type FlapStatus = {
    state: FlapState;
    flap_index: number;
    moving: boolean;
    home_state: boolean;
    count_unexpected_home: number;
    count_missed_home: number;
};

show all the values in a table
-->
    <table>
        <tr>
            <td>State</td>
            <td>{status.state}</td>
        </tr>
        <tr>
            <td>Flap Index</td>
            <td>{status.flap_index}</td>
        </tr>
        <tr>
            <td>Moving</td>
            <td>{status.moving ? 'Yes' : 'No'}</td>
        </tr>
        <tr>
            <td>Home State</td>
            <td>{status.home_state ? 'Home' : 'Not Home'}</td>
        </tr>
        <tr>
            <td>Unexpected Home</td>
            <td>{status.count_unexpected_home}</td>
        </tr>
        <tr>
            <td>Missed Home</td>
            <td>{status.count_missed_home}</td>
        </tr>
    </table>
</div>
