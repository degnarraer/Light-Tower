export class Model_BtDeviceSelector {

    constructor(signalName, wsManager) {
        this.signalName = signalName;
        this.wsManager = wsManager;
        this.deviceList = [];
        this.selectedDevice = null;
        this.wsManager.registerListener(this);
        this.container = this.findDeviceListWidget();
    }

    cleanup() {
        this.wsManager.unregisterListener(this);
    }

    getListnerName() {return this.signalName;}
    onOpen(){}
    onClose(){}
    onError(){}
    onMessage(deviceData) {
        console.debug(`Message Rx for: "${this.signalName}" with device data: ${deviceData}`);
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
        return document.querySelector(`[data-signal="${this.signalName}"]`);
    }

    selectDevice(index) {
        this.selectedDevice = this.deviceList[index];
        this.updateHTML();
        console.log(`Selected Device: ${this.deviceList[index].name}`);
    }

    updateHTML() {
        if (!this.container) {
            console.error(`Container for signal "${this.signalName}" not found.`);
            return;
        }

        this.container.innerHTML = ''; // Clear previous entries

        this.deviceList.forEach((device, index) => {
            const deviceElement = document.createElement('div');
            deviceElement.innerHTML = `
                <strong>Name:</strong> ${device.name} <br>
                <strong>Address:</strong> ${device.address} <br>
                <strong>RSSI:</strong> ${device.rssi}
            `;
            deviceElement.style.padding = '10px';
            deviceElement.style.cursor = 'pointer';
            deviceElement.style.borderBottom = '1px solid #ccc';

            // Highlight the selected device
            if (this.selectedDevice && this.selectedDevice.name === device.name) {
                deviceElement.style.backgroundColor = '#ADD8E6';
            }

            deviceElement.addEventListener('click', () => this.selectDevice(index));
            this.container.appendChild(deviceElement);
        });
    }
}
