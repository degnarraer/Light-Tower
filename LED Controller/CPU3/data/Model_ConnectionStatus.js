export class Model_ConnectionStatus {
    
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
        Disconnected: "Disconnected",
        Connecting: "Connecting",
        Connected: "Connected",
        Disconnecting: "Disconnecting",
        Unknown: "Unknown"
    };

    cleanup() {
        this.wsManager.unregisterListener(this);
    }

    getListnerName() {
        return this.signalName;
    }

    onMessage(newValue) {
        console.debug(`Message Rx for: "${this.signalName}" with value: "${newValue}"`);
        try {
            this.fromString(newValue);
        } catch (e) {
            console.error(`Error handling message: ${e.message}`);
        }
    }

    setValue(newValue, updateWebsocket = true) {
        console.log(`Set Value for Signal: "${this.signalName}" to "${newValue}"`);
        if (Object.values(Model_ConnectionStatus.values).includes(newValue)) {
            this.value = newValue;
            this.updateHTML();
        } else {
            console.error(`"${this.signalName}" Unknown Value: "${newValue}"`);
            throw new Error(`Invalid Value for ${this.signalName}: ${newValue}`);
        }
    }

    getValue() {
        return this.value;
    }

    toString() {
        return Object.values(Model_ConnectionStatus.values).includes(this.value) ? this.value : "Invalid Status";
    }

    fromString(str) {
        console.log(`Parsing string to status: "${str}"`);
        let mappedValue;
        switch (str) {
            case "Disconnected":
            case "0":
                mappedValue = Model_ConnectionStatus.values.Disconnected;
                break;
            case "Connected":
            case "1":
                mappedValue = Model_ConnectionStatus.values.Connected;
                break;
            case "Connecting":
            case "2":
                mappedValue = Model_ConnectionStatus.values.Connecting;
                break;
            case "Disconnecting":
            case "3":
                mappedValue = Model_ConnectionStatus.values.Disconnecting;
                break;
            case "Unknown":
            case "4":
                mappedValue = Model_ConnectionStatus.values.Disconnecting;
                break;
            default:
                mappedValue = str;
                break;
        }
        const validValue = Object.values(Model_ConnectionStatus.values).find(value => value === mappedValue);
        if (validValue) {
            this.setValue(validValue);
        } else {
            console.error(`"${this.signalName}" Invalid string for status: "${str}"`);
            throw new Error(`Invalid status string: ${str}`);
        }
    }
    
    updateHTML() {
        const elementsWithDataValue = document.querySelectorAll(`[data-Signal="${this.signalName}"]`);
        elementsWithDataValue.forEach(element => {
            if (element.tagName === "SPAN") {
                element.innerHTML = this.value;
            } else {
                console.error(`"${this.signalName}" Unsupported Element!`);
            }
        });
    }
}