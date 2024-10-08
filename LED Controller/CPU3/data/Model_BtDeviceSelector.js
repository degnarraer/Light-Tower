export class Model_BtDeviceSelector {

    constructor(signalName, wsManager) {
        this.signalName = signalName;
        this.wsManager = wsManager;
        this.deviceList = [];
        this.selectedDevice = null;
        this.wsManager.registerListener(this);
        this.createDeviceListWidget();
    }

    cleanup() {
        this.wsManager.unregisterListener(this);
    }

    getListenerName() {
        return this.signalName;
    }

    onOpen() {}
    onClose() {}
    onError() {}
    
    onMessage(deviceData) {
        console.debug(`Message Rx for: "${this.signalName}" with device data: ${deviceData}`);
        const parsedData = JSON.parse(deviceData);
        this.updateDeviceList(parsedData);
    }

    updateDeviceList(devices) {
        this.deviceList = devices;
        this.updateHTML();
    }

    createDeviceListWidget() {
        // Create the scrollable view container
        const container = document.createElement('div');
        container.setAttribute('id', `deviceList-${this.signalName}`);
        container.style.overflowY = 'scroll';
        container.style.maxHeight = '300px';
        container.style.border = '1px solid #000';
        container.style.padding = '10px';
        container.style.width = '300px';
        container.style.fontFamily = 'Arial, sans-serif';

        // Add the container to the body or a specific element
        document.body.appendChild(container);
    }

    selectDevice(index) {
        this.selectedDevice = this.deviceList[index];
        this.updateHTML();
        console.log(`Selected Device: ${this.deviceList[index].name}`);
    }

    updateHTML() {
        const container = document.querySelector(`#deviceList-${this.signalName}`);
        container.innerHTML = '';

        this.deviceList.forEach((device, index) => {
            const deviceElement = document.createElement('div');
            deviceElement.textContent = `${device.name} (RSSI: ${device.rssi})`;
            deviceElement.style.padding = '10px';
            deviceElement.style.cursor = 'pointer';
            deviceElement.style.borderBottom = '1px solid #ccc';

            if (this.selectedDevice && this.selectedDevice.name === device.name) {
                deviceElement.style.backgroundColor = '#ADD8E6'; // Highlight selected device
            }

            deviceElement.addEventListener('click', () => this.selectDevice(index));
            container.appendChild(deviceElement);
        });
    }
}
