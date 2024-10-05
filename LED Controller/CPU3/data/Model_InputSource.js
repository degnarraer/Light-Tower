export class Model_InputSource {
    
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
        OFF: 'OFF',
        Microphone: 'Microphone',
        Bluetooth: 'Bluetooth',
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
        if (Object.values(Model_InputSource.values).includes(newValue)) {
            this.value = newValue;
            this.showSourceInputContent();
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
        return this.value;
    }

    fromString(str) {
        this.setValue(Model_InputSource.values[str] || Model_InputSource.values.OFF);
    }
    
    showSourceInputContent() {
        const tabContents = document.querySelectorAll('.selection_tab_content_input_source');
        tabContents.forEach(tabContent => tabContent.classList.remove('active'));

        const contentId = this.getContentIdForValue(this.value);
        if (contentId) {
            document.getElementById(contentId).classList.add('active');
        }
    }

    getContentIdForValue(value) {
        switch (value) {
            case Model_InputSource.values.OFF:
                console.debug(`"${this.signalName}" Show Source Input Content: "OFF"`);
                return 'Sound_Input_Selection_OFF';
            case Model_InputSource.values.Microphone:
                console.debug(`"${this.signalName}" Show Source Input Content: "Microphone"`);
                return 'Sound_Input_Selection_Microphone';
            case Model_InputSource.values.Bluetooth:
                console.debug(`"${this.signalName}" Show Source Input Content: "Bluetooth"`);
                return 'Sound_Input_Selection_Bluetooth';
            default:
                console.error(`"${this.signalName}" Unknown value: "${value}"`);
                return null;
        }
    }
}