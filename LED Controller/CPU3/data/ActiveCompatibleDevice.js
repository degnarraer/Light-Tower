class ActiveCompatibleDevice {
    constructor(name = '', address = '', rssi = 0, lastUpdateTime = 0, timeSinceUpdate = 0) {
        this.name = name;
        this.address = address;
        this.rssi = rssi;
        this.lastUpdateTime = lastUpdateTime;
        this.timeSinceUpdate = timeSinceUpdate;
    }

    static fromString(str) {
        const parts = str.split('|').map(part => part.trim());
        if (parts.length < 5) {
            return new ActiveCompatibleDevice(); // handle error
        }

        const name = parts[0];
        const address = parts[1];
        const rssi = parseInt(parts[2], 10);
        const lastUpdateTime = parseInt(parts[3], 10);
        const timeSinceUpdate = parseInt(parts[4], 10);

        return new ActiveCompatibleDevice(name, address, rssi, lastUpdateTime, timeSinceUpdate);
    }

    toString() {
        return `${this.name} | ${this.address} | ${this.rssi} | ${this.lastUpdateTime} | ${this.timeSinceUpdate}`;
    }

    equals(other) {
        return this.name === other.name && this.address === other.address;
    }
}

export default ActiveCompatibleDevice;