
import { Model_Boolean } from './Model_Boolean.js';

export class Model_DiscoveryMode {
    constructor(signalName, initialValue, wsManager) {
        this.signalName = signalName;
        this.wsManager = wsManager;
        this.wsManager.registerListener(this);
        this.setValue(initialValue, false);
    }

    static values = {
        Discovery_Mode_Started : "Started",
        Discovery_Mode_Stopped : "Stopped",
        Discovery_Mode_Unknown : "Unknown"
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
        if (Object.values(Model_DiscoveryMode.values).includes(newValue)) {
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
                if (!Root.isNull())
                {
                    var Message = JSON.stringify(Root);
                    console.log('ESP32 Web Socket Tx: \'' + Message + '\'');
                    this.wsManager.send(Message);
                }
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
            case Model_DiscoveryMode.values.Discovery_Mode_Started:
                return 'Started';
            case Model_DiscoveryMode.values.Discovery_Mode_Stopped:
                return 'Stopped';
            case Model_DiscoveryMode.values.Discovery_Mode_Unknown:
            default:
                return 'Unknown';
        }
    }

    fromString(str) {
        switch (str) {
            case '0':
            case 'Started':
                this.setValue(Model_DiscoveryMode.Discovery_Mode_Started);
                break;
            case '1':
            case 'Stopped':
                this.setValue(Model_DiscoveryMode.values.Discovery_Mode_Stopped);
                break;
            default:
                this.setValue(Model_DiscoveryMode.values.Discovery_Mode_Unknown);
                break;
        }
    }
    
    spinOn(image) {
        console.log(`Starting spin for image: "${image}"`);
        image.classList.add("spin-image");
    }
      
    spinOff(image) {
        console.log(`Stopping spin for image: "${image}"`);
        image.classList.remove("spin-image");
    }

    updateHTML() { 
        console.log(`Updating HTML for Signal: "${this.signalName}"`);
        var elementsWithDataValue = document.querySelectorAll(`[data-Signal="${this.signalName}"]`);
        if (elementsWithDataValue.length === 0) {
            console.error(`"${this.signalName}": No Signals Found!`);
        }
        
        elementsWithDataValue.forEach((element) => {
            if (element.tagName.toLowerCase() === "img") {
                switch (this.value) {
                    case Model_DiscoveryMode.values.Discovery_Mode_Started:
                        this.spinOn(element);
                        break;
                    case Model_DiscoveryMode.values.Discovery_Mode_Stopped:
                        this.spinOff(element);
                        break;
                    case Model_DiscoveryMode.values.Discovery_Mode_Unknown:
                        this.spinOff(element);
                    default:
                        break;
                }
            } else if (element.tagName.toLowerCase() === "button") {
            } else {
                console.error(`"${this.signalName}" Unsupported Element!`);
            }
        });
    }
}