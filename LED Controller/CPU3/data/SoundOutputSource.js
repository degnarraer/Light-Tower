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

    onMessage(Value) {
        console.log("\"" + this.signalName + '\" Listener received value: \"' + Value + "\"");
        this.setValue(Value);
    }
    
    setValue(newValue, updateWebsocket = true) {
        if (Object.values(SoundOutputSource_Signal.values).includes(newValue)) {
            this.value = newValue;
            this.showSourceOutputContent();
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
            case SoundOutputSource_Signal.values.OFF:
                return 'OFF';
            break;
            case SoundOutputSource_Signal.values.Bluetooth:
                return 'Bluetooth';
            break;
            case SoundOutputSource_Signal.values.Count:
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
            var heading = document.getElementById("mainMenu_Heading");
            // Show the selected tab content
            document.getElementById(contentId).classList.add('active');
        }
    }
}