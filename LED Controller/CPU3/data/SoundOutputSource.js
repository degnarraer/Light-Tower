export class SoundOutputSource_Signal {
    
    constructor(signalName, initialValue, wsManager) {
        this.signalName = signalName;
        this.value = initialValue;
        this.wsManager = wsManager;
        this.wsManager.registerListener(this);
        this.setValue(initialValue);
    }

    static values = {
        OFF: 'OFF',
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
        this.setValue(Value);
    }
    
    setValue(newValue) {
        if (Object.values(SoundOutputSource_Signal.values).includes(newValue)) {
            this.value = newValue;
            this.updateHTML();
        } else {
            throw new Error('Invalid Value');
        }
        this.wsManager.Send_Signal_Value_To_Web_Socket(this.getSignalName(), this.toString());
    }

    getValue() {
        return this.value;
    }

    toString() {
        switch (this.value) {
            case this.values.OFF:
                return 'OFF';
            case this.values.Bluetooth:
                return 'Bluetooth';
            case this.values.Count:
                return 'Count';
            default:
                return 'Unknown';
        }
    }

    fromString(str) {
        switch (str) {
            case 'OFF':
                this.setValue(this.values.OFF);
            case 'Bluetooth':
                this.setValue(this.values.Bluetooth);
            default:
                this.setValue(this.values.OFF);
        }
    }
    
    updateHTML(){
		var elementsWithDataValue = document.querySelectorAll('[data-Signal=\"' + this.getSignalName() + '"\"]');
		elementsWithDataValue.forEach(function(element){
			console.log('handleBTSourceReset Unsupported Element!');
		});
    }
}