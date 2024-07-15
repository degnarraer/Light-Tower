export class Boolean_Signal {
    
    constructor(signalName, initialValue, wsManager) {
        this.signalName = signalName;
        this.value = initialValue;
        this.wsManager = wsManager;
        this.wsManager.registerListener(this);
        this.setValue(initialValue);
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

    onMessage(Value) {
        console.log("\"" + this.signalName + '\" Listener received value: \"' + Value + "\"");
    }

    setValue(newValue) {
        if (Object.values(Boolean_Signal.values).includes(newValue)) {
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
            case this.values.False:
                return '0';
            case this.values.True:
                return '1';
            default:
                return 'Unknown';
        }
    }

    fromString(str) {
        switch (str) {
            case '0':
            case 'false':
            case 'False':
                this.setValue(this.values.False);
            case '1':
            case 'true':
            case 'True':
                this.setValue(this.values.True);
            default:
                this.setValue(this.values.False);
        }
    }
    
    updateHTML(){
		var elementsWithDataValue = document.querySelectorAll('[data-Signal=\"' + this.getSignalName() + '"\"]');
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