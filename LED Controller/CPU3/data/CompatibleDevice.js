class CompatibleDevice {
    constructor(name = '', address = '') {
        this.name = name;
        this.address = address;
    }

    static fromString(str) {
        const parts = str.split('|').map(part => part.trim());
        if (parts.length < 2) {
            return new CompatibleDevice(); // Handle error
        }
        const name = parts[0];
        const address = parts[1];
        return new CompatibleDevice(name, address);
    }

    toString() {
        return `${this.name} | ${this.address}`;
    }

    equals(other) {
        return this.name === other.name && this.address === other.address;
    }
}

export default CompatibleDevice;