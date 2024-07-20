export class SoundOutputSource_Signal {
    
    constructor(signalName, initialValue, wsManager) {
        this.signalName = signalName;
        this.value = initialValue;
        this.wsManager = wsManager;
        this.wsManager.registerListener(this);
        this.setValue(initialValue, false);
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

    onMessage(newValue) {
        console.log(`Message Rx for: "${this.signalName}" with value: "${newValue}"`);
        this.setValue(newValue);
    }
    
    setValue(newValue, updateWebsocket = true) {
        console.log(`Set Value for Signal: "${this.signalName}" to "${newValue}"`);
        if (Object.values(SoundOutputSource_Signal.values).includes(newValue)) {
            this.value = newValue;
            this.showSourceOutputContent();
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
            case SoundOutputSource_Signal.values.OFF:
                return 'OFF';
            break;
            case SoundOutputSource_Signal.values.Bluetooth:
                return 'Bluetooth';
            break;
            default:
                return 'Unknown';
            break;
        }
    }

    fromString(str) {
        switch (str) {
            case 'OFF':
                this.setValue(SoundOutputSource_Signal.values.OFF);
            break;
            case 'Bluetooth':
                this.setValue(SoundOutputSource_Signal.values.Bluetooth);
            break;
            default:
                this.setValue(SoundOutputSource_Signal.values.OFF);
            break;
        }
    }
    
    showSourceOutputContent() {
        var contentId;
        // Hide all tab contents
        var tabContents = document.querySelectorAll('.selection_tab_content_output_source');
        tabContents.forEach(function (tabContent) {
            tabContent.classList.remove('active');
        });
        var validValue = true;
        switch (this.value) {
            case SoundOutputSource_Signal.values.OFF:
                console.log('\"' + this.signalName + '\" Show Source Outout Content: \"OFF\"');
                contentId = 'Sound_Output_Selection_OFF';
            break;
            case SoundOutputSource_Signal.values.Bluetooth:
                console.log('\"' + this.signalName + '\" Show Source Outout Content: \"Bluetooth\"');
                contentId = 'Sound_Output_Selection_Bluetooth';
            break;
            default:
                validValue = false;
            break;
        }
        if(validValue) {
            document.getElementById(contentId).classList.add('active');
        }
    }
}