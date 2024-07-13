

class SoundState {
    static Value = {
        LastingSilenceDetected: 'LastingSilenceDetected',
        SilenceDetected: 'SilenceDetected',
        Sound_Level1_Detected: 'Sound_Level1_Detected',
        Sound_Level2_Detected: 'Sound_Level2_Detected',
        Sound_Level3_Detected: 'Sound_Level3_Detected',
        Sound_Level4_Detected: 'Sound_Level4_Detected',
        Sound_Level5_Detected: 'Sound_Level5_Detected',
        Sound_Level6_Detected: 'Sound_Level6_Detected',
        Sound_Level7_Detected: 'Sound_Level7_Detected',
        Sound_Level8_Detected: 'Sound_Level8_Detected',
        Sound_Level9_Detected: 'Sound_Level9_Detected',
        Sound_Level10_Detected: 'Sound_Level10_Detected',
        Sound_Level11_Detected: 'Sound_Level11_Detected'
    };

    static ToString(state) {
        switch (state) {
            case SoundState.Value.LastingSilenceDetected:
                return "LastingSilenceDetected";
            case SoundState.Value.SilenceDetected:
                return "SilenceDetected";
            case SoundState.Value.Sound_Level1_Detected:
                return "Sound_Level1_Detected";
            case SoundState.Value.Sound_Level2_Detected:
                return "Sound_Level2_Detected";
            case SoundState.Value.Sound_Level3_Detected:
                return "Sound_Level3_Detected";
            case SoundState.Value.Sound_Level4_Detected:
                return "Sound_Level4_Detected";
            case SoundState.Value.Sound_Level5_Detected:
                return "Sound_Level5_Detected";
            case SoundState.Value.Sound_Level6_Detected:
                return "Sound_Level6_Detected";
            case SoundState.Value.Sound_Level7_Detected:
                return "Sound_Level7_Detected";
            case SoundState.Value.Sound_Level8_Detected:
                return "Sound_Level8_Detected";
            case SoundState.Value.Sound_Level9_Detected:
                return "Sound_Level9_Detected";
            case SoundState.Value.Sound_Level10_Detected:
                return "Sound_Level10_Detected";
            case SoundState.Value.Sound_Level11_Detected:
                return "Sound_Level11_Detected";
            default:
                return "Unknown";
        }
    }

    static FromString(str) {
        switch (str) {
            case "LastingSilenceDetected":
                return SoundState.Value.LastingSilenceDetected;
            case "SilenceDetected":
                return SoundState.Value.SilenceDetected;
            case "Sound_Level1_Detected":
                return SoundState.Value.Sound_Level1_Detected;
            case "Sound_Level2_Detected":
                return SoundState.Value.Sound_Level2_Detected;
            case "Sound_Level3_Detected":
                return SoundState.Value.Sound_Level3_Detected;
            case "Sound_Level4_Detected":
                return SoundState.Value.Sound_Level4_Detected;
            case "Sound_Level5_Detected":
                return SoundState.Value.Sound_Level5_Detected;
            case "Sound_Level6_Detected":
                return SoundState.Value.Sound_Level6_Detected;
            case "Sound_Level7_Detected":
                return SoundState.Value.Sound_Level7_Detected;
            case "Sound_Level8_Detected":
                return SoundState.Value.Sound_Level8_Detected;
            case "Sound_Level9_Detected":
                return SoundState.Value.Sound_Level9_Detected;
            case "Sound_Level10_Detected":
                return SoundState.Value.Sound_Level10_Detected;
            case "Sound_Level11_Detected":
                return SoundState.Value.Sound_Level11_Detected;
            default:
                return SoundState.Value.LastingSilenceDetected; // Default or error value
        }
    }
}

export default SoundState;