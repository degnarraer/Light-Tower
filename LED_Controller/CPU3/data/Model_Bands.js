export class Model_Bands {
    constructor(signalName, initialValue, wsManager) {
        this.CreateFFTBars(signalName);
        this.signalName = signalName;
        this.values = initialValue || Array(32).fill(0);
        this.wsManager = wsManager;
        this.wsManager.registerListener(this);
        this.setValue(this.values, false);
        this.debounceDelay = 1000;
        this.updateWebSocketTimeout = null;
    }

    cleanup() {
        this.wsManager.unregisterListener(this);
        if (this.debounceTimer) {
            clearInterval(this.debounceTimer);
        }
    }

    getListnerName() {
        return this.signalName;
    }

    onOpen() {}
    onClose() {}
    onError() {}

    onMessage(newValue) {
        console.debug(`Message Rx for: "${this.signalName}" with value: "${newValue}"`);
        try {
                const values = newValue.split('|').map(parseFloat);
                if (values.length === 32) {
                    this.setValue(values, false);
                } else {
                    console.error(`"${this.signalName}" received invalid data length: ${values.length}`);
                }
        } catch (error) {
            console.error(`"${this.signalName}" failed to process message:`, error);
        }
    }

    CreateFFTBars(DataSignal) {
        const fftContainer = document.querySelector(`.fft-widget[data-Signal="${DataSignal}"]`);
        if (!fftContainer) {
            console.error(`Container with data-Signal="${DataSignal}" not found.`);
            return;
        }
        for (let i = 0; i < 32; i++) {
            const bar = document.createElement('div');
            bar.classList.add('fft-bar');
            
            const barFill = document.createElement('div');
            barFill.classList.add('fft-bar-fill');
            
            bar.appendChild(barFill);
            fftContainer.appendChild(bar);
        }
    }

    setValue(newValues, updateWebsocket = true) {
        console.log(`Set Values for Signal: "${this.signalName}"`, newValues);
        if (Array.isArray(newValues) && newValues.length === 32) {
            this.values = newValues.map((v) => Math.min(Math.max(v, 0), 1)); // Clamp values to 0-1
            this.updateHTML();
        } else {
            console.error(`"${this.signalName}" Invalid Values:`, newValues);
            throw new Error(`Invalid FFT Values for ${this.signalName}`);
        }
        if (updateWebsocket) {
            this.scheduleWebSocketUpdate();
        }
    }

    scheduleWebSocketUpdate() {
        console.log(`Schedule Update: "${this.signalName}"`, this.values);
        if (!this.updateWebSocketTimeout) {
            this.sendWebSocketUpdate();
            this.updateWebSocketTimeout = setTimeout(() => {
                this.sendWebSocketUpdate();
                this.updateWebSocketTimeout = null;
            }, this.debounceDelay);
        }
    }

    sendWebSocketUpdate() {
        console.log(`sendWebSocketUpdate: "${this.signalName}"`, this.values);
        this.Send_Signal_Value_To_Web_Socket(this.getListnerName(), JSON.stringify(this.values));
    }

    Send_Signal_Value_To_Web_Socket(signal, value) {
        if (this.wsManager) {
            if (signal && value) {
                const Root = {
                    SignalValue: {
                        Id: signal.toString(),
                        Value: value.toString(),
                    },
                };
                const Message = JSON.stringify(Root);
                console.log(`ESP32 Web Socket Tx: '${Message}'`);
                this.wsManager.send(Message);
            } else {
                console.error('Invalid Call to Update_Signal_Value_To_Web_Socket!');
            }
        } else {
            console.error('Null wsManager!');
        }
    }

    updateHTML() {
        console.log(`Updating HTML for Signal: "${this.signalName}"`);
        const fftBars = document.querySelectorAll(`[data-Signal="${this.signalName}"] .fft-bar-fill`);
        if (fftBars.length !== 32) {
            console.error(`"${this.signalName}": Expected 32 bars, found ${fftBars.length}!`);
            return;
        }
        this.values.forEach((value, index) => {
            const heightPercent = value * 100; // Convert to percentage
            fftBars[index].style.height = `${heightPercent}%`;
            fftBars[index].style.backgroundColor = `hsl(${Math.round(120 - heightPercent)}, 100%, 50%)`; // Green to red gradient
        });
    }
}
