export class SoundOutputSource {
    static Value = {
        OFF: 'OFF',
        Bluetooth: 'Bluetooth',
        Count: 'Count'
    };

    static ToString(source) {
        switch (source) {
            case SoundOutputSource.Value.OFF:
                return 'OFF';
            case SoundOutputSource.Value.Bluetooth:
                return 'Bluetooth';
            case SoundOutputSource.Value.Count:
                return 'Count';
            default:
                return 'Unknown';
        }
    }

    static FromString(str) {
        switch (str) {
            case 'OFF':
                return SoundOutputSource.Value.OFF;
            case 'Bluetooth':
                return SoundOutputSource.Value.Bluetooth;
            case 'Count':
                return SoundOutputSource.Value.Count;
            default:
                return SoundOutputSource.Value.OFF; // Default or error value
        }
    }
}