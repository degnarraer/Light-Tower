export class SoundInputSource_Signal {
    
    constructor(signalName, initialValue, wsManager) {
        this.signalName = signalName;
        this.value = initialValue;
        this.wsManager = wsManager;
        this.wsManager.registerListener(this);
        this.setValue(initialValue, false);
    }

    static values = {
        OFF: 'OFF',
        Microphone: 'Microphone',
        Bluetooth: 'Bluetooth',
        Count: 'Count'
    };

    cleanup() {
        this.wsManager.unregisterListener(this);
    }

    getSignalName() {
        return this.signalName;
    }

    onMessage(Value) {
        console.log("\"" + this.signalName + '\" Listener received value: \"' + Value + "\"");
    }
    
    setValue(newValue, updateWebsocket = true) {
        if (Object.values(SoundInputSource_Signal.values).includes(newValue)) {
            this.value = newValue;
            this.updateHTML();
        } else {
            throw new Error('Invalid Value');
        }
        if(updateWebsocket){
            this.wsManager.Send_Signal_Value_To_Web_Socket(this.getSignalName(), this.toString());
        }
    }

    getValue() {
        return this.value;
    }

    toString() {
        switch (this.value) {
            case SoundInputSource_Signal.values.OFF:
                return 'OFF';
            break;
            case SoundInputSource_Signal.values.Microphone:
                return 'Microphone';
            break;
            case SoundInputSource_Signal.values.Bluetooth:
                return 'Bluetooth';
            break;
            case SoundInputSource_Signal.values.Count:
                return 'Count';
            break;
            default:
                return 'Unknown';
            break;
        }
    }

    fromString(str) {
        switch (str) {
            case 'OFF':
                this.setValue(SoundInputSource_Signal.values.OFF);
            break;
            case 'Microphone':
                this.setValue(SoundInputSource_Signal.values.Microphone);
            break;
            case 'Bluetooth':
                this.setValue(SoundInputSource_Signal.values.Bluetooth);
            break;
            default:
                this.setValue(SoundInputSource_Signal.values.OFF);
            break;
        }
    }
    
    updateHTML(){
		var elementsWithDataValue = document.querySelectorAll('[data-Signal=\"' + this.getSignalName() + '\"]');
		elementsWithDataValue.forEach(function(element){
			console.log('handleBTSourceReset Unsupported Element!');
		});
    }
}