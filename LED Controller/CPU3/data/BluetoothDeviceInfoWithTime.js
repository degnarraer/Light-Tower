class BTDeviceInfoWithTime {
    constructor(name = '', address = '', timeSinceUpdate = 0, rssi = 0) {
        this.name = name;
        this.address = address;
        this.timeSinceUpdate = timeSinceUpdate;
        this.rssi = rssi;
    }

    static fromString(str) {
        const parts = str.split('|').map(part => part.trim());
        if (parts.length < 4) {
            return new BTDeviceInfoWithTime(); // Handle error
        }
        const name = parts[0];
        const address = parts[1];
        const timeSinceUpdate = parseInt(parts[2], 10);
        const rssi = parseInt(parts[3], 10);
        return new BTDeviceInfoWithTime(name, address, timeSinceUpdate, rssi);
    }

    toString() {
        return `${this.name} | ${this.address} | ${this.timeSinceUpdate} | ${this.rssi}`;
    }

    equals(other) {
        return this.name === other.name && this.address === other.address && this.timeSinceUpdate === other.timeSinceUpdate && this.rssi === other.rssi;
    }

    notEquals(other) {
        return !this.equals(other);
    }
}

export default BTDeviceInfoWithTime;