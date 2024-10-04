export class Controller_Button {
    
    constructor(signalName) {
        this.signalName = signalName;
    }

    static values = {
        Not_Pressed: '0',
        Pressed: '1',
    };
    
    getListnerName() {
        return this.signalName;
    }

    toString() {
        switch(this.value)
        {
            case Not_Pressed:
                return "Not Pressed";
            case Pressed:
                return "Pressed";
            default:
                console.error(`Button: "${this.signalName}" Invalid Value`);
                return "Not Pressed";
        }
    }

    setPressState(pressState) {
        this.value = pressState;
        console.log(`Button: "${this.signalName}" Set value to: "${this.value.toString()}"`);
        this.wsManager.Send_Signal_Value_To_Web_Socket(this.getListnerName(), this.value);
    }
    
}
