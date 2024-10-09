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
            console.error("Error parsing message:", error);
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

    updateHTML() {
        if (!this.container) {
            console.error(`Container for signal "${this.deviceList_signalName}" not found.`);
            return;
        }
    
        // Clear any existing waiting timeout when updateHTML is called
        if (this.waitingTimeoutId) {
            clearTimeout(this.waitingTimeoutId);
            this.waitingTimeoutId = null;
        }
    
        // Clear the container for fresh content
        this.container.innerHTML = '';
    
        // If no devices are found, show the "Searching" animation
        if (this.deviceList.length === 0) {
            if (!this.searchingIntervalId) { // Only start the animation if it's not already running
                let periodCount = 0;
                const baseText = 'Searching';
    
                const updateSearchingText = () => {
                    // Clear the container for the new "Searching" text
                    this.container.innerHTML = '';
    
                    // Create a new paragraph element to show the text
                    const searchingText = document.createElement('p');
                    searchingText.style.textAlign = 'center'; // Optional: Center-align the text
                    searchingText.style.fontStyle = 'italic'; // Optional: Italic for styling
    
                    // Build the "Searching" text with the number of periods
                    let text = baseText;
                    for (let i = 0; i < periodCount; i++) {
                        text += '.';
                    }
                    searchingText.textContent = text;
    
                    // Append the "Searching" text to the container
                    this.container.appendChild(searchingText);
    
                    // Update the period count (looping from 0 to 3)
                    periodCount = (periodCount + 1) % 4;
                };
    
                // Start the animation loop every 1 second
                this.searchingIntervalId = setInterval(updateSearchingText, 1000);
            }
    
            return; // Exit the function early since we're in the "Searching" state
        }
    
        // If devices are found, stop the "Searching" animation
        if (this.searchingIntervalId) {
            clearInterval(this.searchingIntervalId);
            this.searchingIntervalId = null; // Reset the interval ID
        }
    
        // Proceed with updating the table for the device list
        const table = document.createElement('table');
        table.style.width = '100%'; // Make the table take up the full width
        table.style.borderCollapse = 'collapse'; // Optional: Make the table borders collapse
    
        // Create table header
        const thead = document.createElement('thead');
        const headerRow = document.createElement('tr');
    
        // Create Name header
        const nameHeader = document.createElement('th');
        nameHeader.textContent = 'Name';
        nameHeader.style.textAlign = 'left'; // Left align the Name header
        headerRow.appendChild(nameHeader);
    
        // Create RSSI header
        const rssiHeader = document.createElement('th');
        rssiHeader.textContent = 'RSSI';
        rssiHeader.style.textAlign = 'right'; // Right align the RSSI header
        headerRow.appendChild(rssiHeader);
    
        thead.appendChild(headerRow);
        table.appendChild(thead);
    
        // Create table body
        const tbody = document.createElement('tbody');
    
        this.deviceList.forEach((device, index) => {
            const name = device.Name || 'Unknown Name';
            const rssi = device.RSSI || 'Unknown RSSI';
    
            // Create a row for each device
            const row = document.createElement('tr');
    
            // Alternate row colors: light gray for even rows, white for odd rows
            row.style.backgroundColor = index % 2 === 0 ? '#f2f2f2' : '#ffffff';
    
            // Create and configure the name cell
            const nameCell = document.createElement('td');
            nameCell.textContent = name;
            nameCell.style.padding = '5px';
            nameCell.style.textAlign = 'left'; // Left-align the Name column
            row.appendChild(nameCell);
    
            // Create and configure the RSSI cell
            const rssiCell = document.createElement('td');
            rssiCell.textContent = rssi;
            rssiCell.style.padding = '5px';
            rssiCell.style.textAlign = 'right'; // Right-align the RSSI column
            row.appendChild(rssiCell);
    
            // Highlight the selected device
            if (this.selectedDevice && this.selectedDevice.Name === device.Name) {
                row.style.backgroundColor = '#ADD8E6'; // Light blue background for selected row
            }
    
            // Add a click listener for selecting a device
            row.style.cursor = 'pointer';
            row.addEventListener('click', () => this.selectDevice(index));
    
            tbody.appendChild(row);
        });
    
        table.appendChild(tbody);
        this.container.appendChild(table); // Append the table to the container
    
        // Start the waiting timeout to display "Waiting" if updateHTML is not called for 5 seconds
        this.waitingTimeoutId = setTimeout(() => {
            this.container.innerHTML = ''; // Clear current content
            const waitingText = document.createElement('p');
            waitingText.textContent = 'Waiting...';
            waitingText.style.textAlign = 'center'; // Optional: Center-align the text
            waitingText.style.fontStyle = 'italic'; // Optional: Italic for styling
            this.container.appendChild(waitingText);
        }, WAITING_TIME);
    }
}
