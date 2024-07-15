export class SoundInputSource_Signal {
    
    constructor(signalName, initialValue, wsManager) {
        this.signalName = signalName;
        this.value = initialValue;
        this.wsManager = wsManager;
        this.wsManager.registerListener(this);
        this.setValue(initialValue, false);
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

    onMessage(Value) {
        console.log("\"" + this.signalName + '\" Listener received value: \"' + Value + "\"");
    }

    setValue(newValue, updateWebsocket = true) {
        console.log('\"' + this.signalName + '\" Set Value: \"' + newValue + '\"');
        if (Object.values(SoundInputSource_Signal.values).includes(newValue)) {
            this.value = newValue;
            this.showSourceInputContent();
        } else {
            throw new Error('Invalid Value');
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
            case SoundInputSource_Signal.values.OFF:
                return 'OFF';
            break;
            case SoundInputSource_Signal.values.Microphone:
                return 'Microphone';
            break;
            case SoundInputSource_Signal.values.Bluetooth:
                return 'Bluetooth';
            break;
            case SoundInputSource_Signal.values.Count:
                return 'Count';
            break;
            default:
                return 'Unknown';
            break;
        }
    }

    fromString(str) {
        switch (str) {
            case 'OFF':
                this.setValue(SoundInputSource_Signal.values.OFF);
            break;
            case 'Microphone':
                this.setValue(SoundInputSource_Signal.values.Microphone);
            break;
            case 'Bluetooth':
                this.setValue(SoundInputSource_Signal.values.Bluetooth);
            break;
            default:
                this.setValue(SoundInputSource_Signal.values.OFF);
            break;
        }
    }
    
    showSourceInputContent() {
        var contentId;
        // Hide all tab contents
        var tabContents = document.querySelectorAll('.selection_tab_content_input_source');
        tabContents.forEach(function (tabContent) {
            tabContent.classList.remove('active');
        });
        var validValue = true;
        switch (this.value) {
            case SoundInputSource_Signal.values.OFF:
                console.log('\"' + this.signalName + '\" Show Source Input Content: \"OFF\"');
                contentId = 'Sound_Input_Selection_OFF';
            break;
            case SoundInputSource_Signal.values.Microphone:
                console.log('\"' + this.signalName + '\" Show Source Input Content: \"Microphone\"');
                contentId = 'Sound_Input_Selection_Microphone';
            break;
            case SoundInputSource_Signal.values.Bluetooth:
                console.log('\"' + this.signalName + '\" Show Source Input Content: \"Bluetooth\"');
                contentId = 'Sound_Input_Selection_Bluetooth';
            break;
            default:
                validValue = false;
            break;
        }
        if(validValue) {
            var heading = document.getElementById("mainMenu_Heading");
            // Show the selected tab content
            document.getElementById(contentId).classList.add('active');
        }
    }
}