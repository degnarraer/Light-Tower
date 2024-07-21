export class SoundInputSource_Signal {
    
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

    getSignalName() {
        return this.signalName;
    }

    onMessage(newValue) {
        console.debug(`Message Rx for: "${this.signalName}" with value: "${newValue}"`);
        this.setValue(newValue);
    }

    setValue(newValue, updateWebsocket = true) {
        console.log(`Set Value for Signal: "${this.signalName}" to "${newValue}"`);
        if (Object.values(SoundInputSource_Signal.values).includes(newValue)) {
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
        return this.value;
    }

    fromString(str) {
        this.setValue(SoundInputSource_Signal.values[str] || SoundInputSource_Signal.values.OFF);
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
            case SoundInputSource_Signal.values.OFF:
                console.debug(`"${this.signalName}" Show Source Input Content: "OFF"`);
                return 'Sound_Input_Selection_OFF';
            case SoundInputSource_Signal.values.Microphone:
                console.debug(`"${this.signalName}" Show Source Input Content: "Microphone"`);
                return 'Sound_Input_Selection_Microphone';
            case SoundInputSource_Signal.values.Bluetooth:
                console.debug(`"${this.signalName}" Show Source Input Content: "Bluetooth"`);
                return 'Sound_Input_Selection_Bluetooth';
            default:
                console.error(`"${this.signalName}" Unknown value: "${value}"`);
                return null;
        }
    }
}