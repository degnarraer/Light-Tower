export class Boolean_Signal {
    
    constructor(signalName, initialValue, wsManager) {
        this.signalName = signalName;
        this.value = initialValue;
        this.wsManager = wsManager;
        this.wsManager.registerListener(this);
        this.setValue(initialValue, false);
        this.debounceDelay = 1000;
        this.updateWebSocketTimeout = null;
    }

    static values = {
        False: '0',
        True: '1',
        Count: 'Count'
    };

    cleanup() {
        this.wsManager.unregisterListener(this);
        if (this.debounceTimer) {
            clearInterval(this.debounceTimer);
        }
    }

    getSignalName() {
        return this.signalName;
    }

    onMessage(newValue) {
        console.debug(`Message Rx for: "${this.signalName}" with value: "${newValue}"`);
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
            this.scheduleWebSocketUpdate();
        }
    }

    scheduleWebSocketUpdate() {
        console.log(`Schedule Update: "${this.signalName}" to "${this.value}"`);
        if (!this.updateWebSocketTimeout) {
            this.sendWebSocketUpdate();
            this.updateWebSocketTimeout = setTimeout(() => {
                this.sendWebSocketUpdate();
                this.updateWebSocketTimeout = null;
            }, this.debounceDelay);
        }
    }

    sendWebSocketUpdate() {
        console.log(`sendWebSocketUpdate: "${this.signalName}" to "${this.value}"`);
        this.wsManager.Send_Signal_Value_To_Web_Socket(this.getSignalName(), this.toString());
    }

    getValue() {
        return this.value;
    }

    toString() {
        switch (this.value) {
            case Boolean_Signal.values.False:
                return '0';
            case Boolean_Signal.values.True:
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
    
    updateHTML() {
        console.log(`Updating HTML for Signal: "${this.signalName}"`);
        var elementsWithDataValue = document.querySelectorAll(`[data-Signal="${this.signalName}"]`);
        if(elementsWithDataValue.Count === 0){
            console.error(`"${this.signalName}": No Signals Found!`);
        }
        elementsWithDataValue.forEach((element) => {
            if (element.tagName.toLowerCase() === "input" && element.type.toLowerCase() === "checkbox") {
                element.checked = this.value === Boolean_Signal.values.True;
                console.log(`"${this.signalName}" Controlled CheckBox "${element.id}" Updated to: "${this.value}"`);
            } else {
                console.error(`"${this.signalName}" Unsupported Element!`);
            }
        });
    }
}
