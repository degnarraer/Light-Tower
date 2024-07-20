export class Boolean_Signal {
    
    constructor(signalName, initialValue, wsManager) {
        this.signalName = signalName;
        this.value = initialValue;
        this.wsManager = wsManager;
        this.wsManager.registerListener(this);
        this.setValue(initialValue, false);
    }

    static values = {
        False: '0',
        True: '1',
        Count: 'Count'
    };

    cleanup() {
        this.wsManager.unregisterListener(this);
    }

    getSignalName() {
        return this.signalName;
    }

    onMessage(newValue) {
        console.log(`Message Rx for: "${this.signalName}" with value: "${newValue}"`);
        this.setValue(newValue);
    }
    
    setValue(newValue, updateWebsocket = true) {
        console.log(`Set Value for Signal: "${this.signalName}" to "${newValue}"`);
        if (Object.values(Boolean_Signal.values).includes(newValue)) {
            this.value = newValue;
            this.updateHTML();
        } else {
            console.error(`"${this.signalName}" Unknown Value: "${newValue}"`);
            throw new Error(`Invalid Value for ${this.signalName}: ${newValue}`);
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
            case Boolean_Signal.values.False:
                return '0';
            break;
            case Boolean_Signal.values.True:
                return '1';
            break;
            default:
                return 'Unknown';
            break;
        }
    }

    fromString(str) {
        switch (str) {
            case '0':
            case 'false':
            case 'False':
                this.setValue(Boolean_Signal.values.False);
            break;
            case '1':
            case 'true':
            case 'True':
                this.setValue(Boolean_Signal.values.True);
            break;
            default:
                this.setValue(Boolean_Signal.values.False);
            break;
        }
    }
    
    updateHTML(){
		var elementsWithDataValue = document.querySelectorAll('[data-Signal=\"' + this.getSignalName() + '\"]');
		elementsWithDataValue.forEach(function(element){
			if(element.tagName.toLowerCase() === "input" && element.type.toLowerCase() === "checkbox"){
				if(this.value == this.values.True){
					element.checked = true;
				}else{
					element.checked = false;
				}
			} else {
				console.log('handleBTSourceReset Unsupported Element!');
			}
		});
    }
}