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
            <th>State</th>
            <td>{status.state}</td>
        </tr>
        <tr>
            <th>Flap Index</th>
            <td>{status.flap_index}</td>
        </tr>
        <tr>
            <th>Moving</th>
            <td>{status.moving ? 'Yes' : 'No'}</td>
        </tr>
        <tr>
            <th>Home State</th>
            <td>{status.home_state ? 'Home' : 'Not Home'}</td>
        </tr>
        <tr>
            <th>Unexpected<br />Home</th>
            <td>{status.count_unexpected_home}</td>
        </tr>
        <tr>
            <th>Missed<br />Home</th>
            <td>{status.count_missed_home}</td>
        </tr>
    </table>
</div>

<style>
    .flap_status {
        text-transform: none;
    }
    .flap_status th {
        text-align: right;
    }
</style>
