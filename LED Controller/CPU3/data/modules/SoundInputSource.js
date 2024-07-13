export class SoundInputSource {
    static Value = {
        OFF: 'OFF',
        Microphone: 'Microphone',
        Bluetooth: 'Bluetooth',
        Count: 'Count'
    };

    static ToString(source) {
        switch (source) {
            case SoundInputSource.Value.OFF:
                return 'OFF';
            case SoundInputSource.Value.Microphone:
                return 'Microphone';
            case SoundInputSource.Value.Bluetooth:
                return 'Bluetooth';
            case SoundInputSource.Value.Count:
                return 'Count';
            default:
                return 'Unknown';
        }
    }

    static FromString(str) {
        switch (str) {
            case 'OFF':
                return SoundInputSource.Value.OFF;
            case 'Microphone':
                return SoundInputSource.Value.Microphone;
            case 'Bluetooth':
                return SoundInputSource.Value.Bluetooth;
            case 'Count':
                return SoundInputSource.Value.Count;
            default:
                return SoundInputSource.Value.OFF; // Default or error value
        }
    }
}