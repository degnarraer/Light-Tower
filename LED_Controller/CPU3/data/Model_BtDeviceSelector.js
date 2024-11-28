const WAITING_TIME = 5000;
export class Model_BtDeviceSelector {
    
    constructor(deviceList_signalName, selection_signalName, wsManager) {
        this.deviceList_signalName = deviceList_signalName;
        this.selection_signalName = selection_signalName;
        this.wsManager = wsManager;
        this.deviceList = [];
        this.selectedDevice = null;
        this.wsManager.registerListener(this);
        this.container = this.findDeviceListWidget();
        this.searchingIntervalId = null;
        this.waitingTimeoutId = null;
    }

    cleanup() {
        this.wsManager.unregisterListener(this);
    }

    getListnerName() {return this.deviceList_signalName;}
    onOpen(){}
    onClose(){}
    onError(){}
    onMessage(deviceData) {
        console.debug(`Message Rx for: "${this.deviceList_signalName}" with device data: ${deviceData}`);
        try {
            const parsedData = JSON.parse(deviceData);
    
            // Log the parsed data for debugging
            console.debug("Parsed Data:", parsedData);
    
            // Check if Devices array exists
            if (parsedData.Devices) {
                this.updateDeviceList(parsedData.Devices);
            } else {
                console.error("Devices array is undefined:", parsedData);
            }
        } catch (error) {
            console.error("Error parsing message: ", deviceData, " Error: " , error );
        }
    }

    updateDeviceList(devices) {
        if (Array.isArray(devices)) {
            this.deviceList = devices; // Assign the devices array
        } else {
            console.error("Expected an array of devices.");
            this.deviceList = []; // Reset to an empty array if not valid
        }
        this.updateHTML();
    }

    findDeviceListWidget() {
        return document.querySelector(`[data-signal="${this.deviceList_signalName}"]`);
    }

    selectDevice(index) {
        this.selectedDevice = this.deviceList[index];
        this.updateHTML();
        console.log(`Selected Device Name: "${this.selectedDevice.Name}" Address: "${this.selectedDevice.Address}"`);
        this.scheduleWebSocketUpdate();
    }

    scheduleWebSocketUpdate() {
        console.log(`ESP32: ESP32: Schedule Update: "${this.selection_signalName}" to Name: "${this.selectedDevice.Name}" Address: "${this.selectedDevice.Address}"`);
        if (!this.updateWebSocketTimeout) {
            this.sendWebSocketUpdate();
            this.updateWebSocketTimeout = setTimeout(() => {
                this.sendWebSocketUpdate();
                this.updateWebSocketTimeout = null;
            }, this.debounceDelay);
        }
    }

    toString(){
        return `${this.selectedDevice.Name},${this.selectedDevice.Address}`;
    }

    sendWebSocketUpdate() {
        console.log(`sendWebSocketUpdate: "${this.selection_signalName}" to Name: "${this.selectedDevice.Name}" Address: "${this.selectedDevice.Address}"`);
        this.Send_Signal_Value_To_Web_Socket(this.selection_signalName, this.toString());
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
    updateHTML() {
        if (!this.container) {
            console.error(`Container for signal "${this.deviceList_signalName}" not found.`);
            return;
        }
    
        if (this.waitingTimeoutId) {
            clearTimeout(this.waitingTimeoutId);
            this.waitingTimeoutId = null;
        }
    
        this.container.innerHTML = '';
    
        if (this.deviceList.length === 0) {
            if (!this.searchingIntervalId) {
                let periodCount = 0;
                const baseText = 'Searching';
    
                const updateSearchingText = () => {
                    this.container.innerHTML = '';
                    const searchingText = document.createElement('p');
                    searchingText.style.textAlign = 'center';
                    searchingText.style.fontStyle = 'italic';
                    let text = baseText;
                    for (let i = 0; i < periodCount; i++) {
                        text += '.';
                    }
                    searchingText.textContent = text;
                    this.container.appendChild(searchingText);
                    periodCount = (periodCount + 1) % 4;
                };
    
                this.searchingIntervalId = setInterval(updateSearchingText, 1000);
            }
    
            return;
        }
    
        if (this.searchingIntervalId) {
            clearInterval(this.searchingIntervalId);
            this.searchingIntervalId = null;
        }
    
        const table = document.createElement('table');
        table.style.width = '100%';
        table.style.borderCollapse = 'collapse';
    
        const thead = document.createElement('thead');
        const headerRow = document.createElement('tr');
    
        const nameHeader = document.createElement('th');
        nameHeader.textContent = 'Name';
        nameHeader.style.textAlign = 'left';
        headerRow.appendChild(nameHeader);
    
        const rssiHeader = document.createElement('th');
        rssiHeader.textContent = 'Signal Strength';
        rssiHeader.style.textAlign = 'right';
        headerRow.appendChild(rssiHeader);
    
        thead.appendChild(headerRow);
        table.appendChild(thead);
    
        const tbody = document.createElement('tbody');
    
        this.deviceList.forEach((device, index) => {
            const name = device.Name || 'Unknown Name';
            const rssi = device.RSSI || 0; // Set default RSSI if missing
    
            const row = document.createElement('tr');
            row.style.backgroundColor = index % 2 === 0 ? '#f2f2f2' : '#ffffff';
    
            const nameCell = document.createElement('td');
            nameCell.textContent = name;
            nameCell.style.padding = '5px';
            nameCell.style.textAlign = 'left';
            row.appendChild(nameCell);
    
            // Create the RSSI (Signal Strength) cell
            const rssiCell = document.createElement('td');
            rssiCell.style.padding = '5px';
            rssiCell.style.textAlign = 'right';
    
            // Create the image based on the RSSI value
            const rssiImage = document.createElement('img');
            rssiImage.style.height = '20px'; // Adjust the size as needed
    
            if (rssi <= -80) {
                rssiImage.src = './Images/signal-1.svg';  // Weak signal
            } else if (rssi <= -60) {
                rssiImage.src = './Images/signal-2.svg';
            } else if (rssi <= -40) {
                rssiImage.src = './Images/signal-3.svg';
            } else if (rssi <= -20) {
                rssiImage.src = './Images/signal-4.svg';
            } else {
                rssiImage.src = './Images/signal-5.svg'; // Strong signal
            }
    
            rssiCell.appendChild(rssiImage);
            row.appendChild(rssiCell);
    
            if (this.selectedDevice && this.selectedDevice.Name === device.Name) {
                row.style.backgroundColor = '#ADD8E6';
            }
    
            row.style.cursor = 'pointer';
            row.addEventListener('click', () => this.selectDevice(index));
    
            tbody.appendChild(row);
        });
    
        table.appendChild(tbody);
        this.container.appendChild(table);
    
        this.waitingTimeoutId = setTimeout(() => {
            this.container.innerHTML = '';
            const waitingText = document.createElement('p');
            waitingText.textContent = 'Waiting...';
            waitingText.style.textAlign = 'center';
            waitingText.style.fontStyle = 'italic';
            this.container.appendChild(waitingText);
        }, WAITING_TIME);
    }
}
