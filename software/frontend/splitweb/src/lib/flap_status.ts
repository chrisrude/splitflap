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

// create default FlapStatus
export const createDefaultFlapStatus = (): FlapStatus => ({
    state: 'unknown',
    flap_index: 0,
    moving: false,
    home_state: false,
    count_unexpected_home: 0,
    count_missed_home: 0,
});

export const isFlapOk = (flap_status: FlapStatus): boolean => {
    return flap_status.state === 'normal' || flap_status.state === 'unknown';
}
