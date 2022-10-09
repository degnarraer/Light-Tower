#include "WireSlaveRequest.h"

WireSlaveRequest::WireSlaveRequest(TwoWire &wire, uint8_t address, uint16_t responseLength)
    :wire_(wire)
    ,address_(address)
    ,readLength_(responseLength + 4)    // start, length, crc and end bytes
    ,retryDelay_(10)
    ,maxAttempts_(5)
    ,lastStatus_(NONE)
{
}

bool WireSlaveRequest::request(uint8_t address)
{
    if (address != 0) {
        address_ = address;
    }

    uint8_t attempts = 0;

    WireUnpacker unpacker;

    bool sendTrigger = true;

    while (attempts < maxAttempts_)
	{
        if (sendTrigger) {
			Serial << "SendTrigger\n";
            triggerUpdate();
            sendTrigger = false;
        }

        // wait until slave fills its output buffer
		vTaskDelay(10000 / portTICK_PERIOD_MS);
        //delay(retryDelay_ * (attempts + 1));

        uint8_t returned = wire_.requestFrom(address_, readLength_);
        if (returned == 0) {
			Serial << "SlaveNotFound\n";
            lastStatus_ = SLAVE_NOT_FOUND;
            return false;
        }

        while (wire_.available()) {
			Serial << "Reading\n";
            uint8_t c = wire_.read();
            unpacker.write(c);
        }

        if (unpacker.available()) {
			Serial << "CompletePacketRead\n";
            // a complete packet was read
            break;
        }
        else if (unpacker.hasError()) {
			Serial << "HasError\n";
            // retry request
            unpacker.reset();
            sendTrigger = true;
        }

        ++attempts;
    }

    if (attempts == maxAttempts_) {
        if (unpacker.hasError()) {
			Serial << "PacketError\n";
            lastStatus_ = PACKET_ERROR;
        }
        else {
			Serial << "MaxAttemptsError\n";
            // not sure what could lead here
            lastStatus_ = MAX_ATTEMPTS;
        }
        return false;
    }

    // copy payload bytes to rxBuffer
    rxIndex_ = 0;
    while (unpacker.available() && (rxIndex_ < UNPACKER_BUFFER_LENGTH)) {
		Serial << "Receiving\n";
        rxBuffer_[rxIndex_] = unpacker.read();
        ++rxIndex_;
    }

    rxLength_ = rxIndex_;
    rxIndex_ = 0;
    lastStatus_ = PACKET_READ;

    return true;
}

String WireSlaveRequest::lastStatusToString() const
{
    switch (lastStatus_) {
    case NONE: return "none";
    case PACKET_READ: return "packet read";
    case SLAVE_NOT_FOUND: return "slave not found";
    case PACKET_ERROR: return "packet error";
    case MAX_ATTEMPTS: return "max attempts";
    default: return "unknown";
    }
}

size_t WireSlaveRequest::available()
{
    if (lastStatus_ != PACKET_READ) {
        return 0;
    }

    return rxLength_ - rxIndex_;
}

int WireSlaveRequest::read()
{
    int value = -1;
    if (lastStatus_ == PACKET_READ && rxIndex_ < rxLength_) {
        value = rxBuffer_[rxIndex_];
        ++rxIndex_;
    }
    return value;
}

void WireSlaveRequest::triggerUpdate()
{
    WirePacker packer;
    packer.end();

    wire_.beginTransmission(address_);
    while (packer.available()) {
        wire_.write(packer.read());
    }
    wire_.endTransmission();
}
