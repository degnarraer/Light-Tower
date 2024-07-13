class MuteState {
    static Value = {
        Mute_State_Un_Muted: 'Mute_State_Un_Muted',
        Mute_State_Muted: 'Mute_State_Muted'
    };

    static ToString(state) {
        switch (state) {
            case MuteState.Value.Mute_State_Un_Muted:
                return "Mute_State_Un_Muted";
            case MuteState.Value.Mute_State_Muted:
                return "Mute_State_Muted";
            default:
                return "Unknown";
        }
    }

    static FromString(str) {
        switch (str) {
            case "Mute_State_Un_Muted":
                return MuteState.Value.Mute_State_Un_Muted;
            case "Mute_State_Muted":
                return MuteState.Value.Mute_State_Muted;
            default:
                return MuteState.Value.Mute_State_Un_Muted; // Default or error value
        }
    }
}

export default MuteState;