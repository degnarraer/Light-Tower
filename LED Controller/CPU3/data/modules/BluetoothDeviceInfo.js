class BTDeviceInfo {
    constructor(name = '', address = '', rssi = 0) {
        this.name = name;
        this.address = address;
        this.rssi = rssi;
    }

    static fromString(str) {
        const parts = str.split('|').map(part => part.trim());
        if (parts.length < 3) {
            return new BTDeviceInfo(); // Handle error
        }
        const name = parts[0];
        const address = parts[1];
        const rssi = parseInt(parts[2], 10);
        return new BTDeviceInfo(name, address, rssi);
    }

    toString() {
        return `${this.name} | ${this.address} | ${this.rssi}`;
    }

    equals(other) {
        return this.name === other.name && this.address === other.address && this.rssi === other.rssi;
    }
}

export default BTDeviceInfo;