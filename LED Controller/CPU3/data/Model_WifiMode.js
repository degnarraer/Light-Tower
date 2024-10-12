import { showContent } from './main.js';
import { hideContent } from './main.js';

export class Model_WifiMode {
    
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
        Station: 'Station',
        AccessPoint: 'AccessPoint',
        Unknown: 'Unknown',
        Count: 'Count'
    };

    cleanup() {
        this.wsManager.unregisterListener(this);
    }

    getListnerName() {return this.signalName;}
    onOpen(){}
    onClose(){}
    onError(){}
    onMessage(newValue) {
        console.debug(`Message Rx for: "${this.signalName}" with value: "${newValue}"`);
        this.setValue(newValue);
    }

    setValue(newValue, updateWebsocket = true) {
        console.log(`ESP32 Model: Set Value for Signal: "${this.signalName}" to "${newValue}"`);
        if (Object.values(Model_WifiMode.values).includes(newValue)) {
            this.value = newValue;
            this.updateHTML();
            this.updateUIVisibility(newValue);
        } else {
            console.error(`"${this.signalName}" Unknown Value: "${newValue}"`);
            throw new Error(`Invalid Value for ${this.signalName}: ${newValue}`);
        }
        if(updateWebsocket){
            this.scheduleWebSocketUpdate();
        }
    }

    updateUIVisibility(value) {
        switch (value) {
            case Model_WifiMode.values.Station:
                showContent("screen-content", "station");
                hideContent("screen-content", "access point");
                break;
            case Model_WifiMode.values.AccessPoint:
                showContent("screen-content", "station");
                showContent("screen-content", "access point");
                break;
            case Model_WifiMode.values.Unknown:
                hideContent("screen-content", "access point");
                hideContent("screen-content", "station");
                break;
        }
    }

    scheduleWebSocketUpdate() {
        console.log(`ESP32: Schedule Update: "${this.signalName}" to "${this.value}"`);
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
        this.Send_Signal_Value_To_Web_Socket(this.getListnerName(), this.toString());
    }

    Send_Signal_Value_To_Web_Socket(signal, value) {
        if (this.wsManager) {
            if (signal && value) {
                var Root = {};
                Root.SignalValue = {};
                Root.SignalValue.Id = signal.toString();
                Root.SignalValue.Value = value.toString();
                var Message = JSON.stringify(Root);
                console.log('ESP32 Web Socket Tx: \'' + Message + '\'');
                this.wsManager.send(Message);
            } else {
                console.error('Invalid Call to Update_Signal_Value_To_Web_Socket!');
            }
        } else {
            console.error('Null wsManager!');
        }
    }

    getValue() {
        return this.value;
    }

    toString() {
        switch (this.value) {
            case Model_WifiMode.values.Station:
                return 'Station';
            case Model_WifiMode.values.AccessPoint:
                return 'AccessPoint';
            default:
                return 'Unknown';
        }
    }

    fromString(str) {
        switch (str) {
            case '0':
            case 'Station':
                this.setValue(Model_WifiMode.values.Station);
                break;
            case '1':
            case 'AccessPoint':
                this.setValue(Model_WifiMode.values.AccessPoint);
                break;
            default:
                this.setValue(Model_WifiMode.values.Unknown);
                break;
        }
    }
    
    updateHTML() { 
        console.log(`Updating HTML for Signal: "${this.signalName}"`);
        var elementsWithDataValue = document.querySelectorAll(`[data-Signal="${this.signalName}"]`);
        if (elementsWithDataValue.length === 0) {
            console.error(`"${this.signalName}": No Signals Found!`);
        }
        
        elementsWithDataValue.forEach((element) => {
            if (element.tagName.toLowerCase() === "input" && element.type.toLowerCase() === "radio") {
                // Check if this radio button's value matches the current model value
                element.checked = (element.value === this.value);
                console.log(`"${this.signalName}" Controlled RadioButton "${element.id}" Updated to: "${this.value}"`);
            } else {
                console.error(`"${this.signalName}" Unsupported Element!`);
            }
        });
    }
}