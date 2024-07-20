export class NumericalValue_Signal {
    
    constructor(signalName, initialValue, wsManager) {
        this.signalName = signalName;
        this.value = initialValue;
        this.wsManager = wsManager;
        this.wsManager.registerListener(this);
        this.setValue(initialValue, false);this.
        debounceDelay = 500;
        this.lastUpdate = 0;
        this.updateWebSocketTimeout = null;
    }

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
        if (!isNaN(newValue) && !isNaN(parseFloat(newValue))) {
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
        if (!this.updateWebSocketTimeout) {
            this.wsManager.Send_Signal_Value_To_Web_Socket(this.getSignalName(), this.toString());
            this.updateWebSocketTimeout = setTimeout(() => {
                this.wsManager.Send_Signal_Value_To_Web_Socket(this.getSignalName(), this.toString());
                this.updateWebSocketTimeout = null;
            }, this.debounceDelay);
        }
    }

    getValue() {
        return this.value;
    }

    toString() {
        return this.value.toString();
    }

    fromString(str) {
        this.setValue(Number(str));
    }
    
    updateHTML() {
        const elementsWithDataValue = document.querySelectorAll(`[data-Signal="${this.signalName}"]`);
        elementsWithDataValue.forEach(element => {
            if (element.hasAttribute("value")) {
                element.value = this.value;
            } else if (element.childNodes.length > 0) {
                element.innerHTML = this.value;
            } else {
                console.log(`handleAmplitudeGain Unsupported Element: ${element.id}`);
            }
        });
    }
}