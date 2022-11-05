#include "I2cBus.h"

#include <cstddef>
#include <cstring>
#include <iostream>

#include "I2CDevImpl.h"

namespace {
inline int16_t MergeTwoByteInInt16(std::byte hsb, std::byte lsb) {
    auto result = static_cast<int16_t>(std::to_integer<uint16_t>(hsb) << 8 | std::to_integer<uint16_t>(lsb));
    return result;
}

inline uint16_t MergeTwoByteInUint16(std::byte hsb, std::byte lsb) {
    return static_cast<uint16_t>(MergeTwoByteInInt16(hsb, lsb));
}
} // namespace

I2CBus::I2CBus(uint32_t busNumber, int32_t deviceAddress)
    : _deviceAddress(deviceAddress) {
    _pimpl = I2CDeviceImpl::Instance(busNumber);
}

/**
 * Check is bus open
 * @return true or false
 */
bool I2CBus::IsOpen() const {
    bool ret = false;
    if (_pimpl) {
        ret = _pimpl->IsOpen();
    }
    return ret;
}

/**
 * Write data and Read data operations
 * @param txBuf
 * @param rxBuf
 * @param bytesToTransfer
 * @param bytesToReceive
 * @return pair {count transfered, ocunt received}
 */
std::pair<int32_t, int32_t> I2CBus::WriteRead(std::byte* txBuf,
                                              std::byte* rxBuf,
                                              size_t bytesToTransfer,
                                              size_t bytesToReceive) {
    auto ret = std::make_pair(-1, -1);
    if (_pimpl && _pimpl->IsOpen()) {
        this->ChangeCommunicationAddress();
        ret = _pimpl->WriteRead(txBuf, rxBuf, bytesToTransfer, bytesToReceive);
    }
    return ret;
}

/**
 * Write data operation
 * @param txBuf
 * @param bytesToTransfer
 * @return
 */
int32_t I2CBus::Write(std::byte* txBuf, size_t bytesToTransfer) {
    int32_t ret = -1;
    if (_pimpl && _pimpl->IsOpen()) {
        this->ChangeCommunicationAddress();
        ret = _pimpl->Write(txBuf, bytesToTransfer);
    }
    return ret;
}

/**
 * Read data operation
 * @param rxBuf
 * @param bytesToTransfer
 * @return
 */
int32_t I2CBus::Read(std::byte* rxBuf, size_t bytesToTransfer) {
    int32_t ret = -1;
    if (_pimpl && _pimpl->IsOpen()) {
        this->ChangeCommunicationAddress();
        ret = _pimpl->Read(rxBuf, bytesToTransfer);
    }
    return ret;
}

/**
 * Change communication mode
 * @param mode
 */
void I2CBus::ChangeCommunicationMode(uint32_t mode) {
    _pimpl->SetCommunicationMode(mode);
}

/**
 * Change communication address
 */
void I2CBus::ChangeCommunicationAddress() {
    _pimpl->SetCommunicationAddress(_deviceAddress);
}

/**
 * Read a single bit from an 8-bit device register.
 * @param regAddr Register regAddr to read from
 * @param bitNum Bit position to read (0-7)
 * @param data Container for single bit value
 * @return Status of read operation (count read bytes or BUS_TRANSFER_ERROR)
 */
int32_t I2CBus::ReadBit(uint8_t reg, uint8_t bitNum, std::byte* data) {
    std::byte read_byte{0};
    auto countRead = ReadByte(reg, &read_byte);
    if (countRead != -1 && countRead != 0) {
        *data = read_byte & (std::byte{1} << bitNum);
    }

    return countRead;
}

/**
 * Read multiple bits from an 8-bit device register.
 * @param regAddr Register regAddr to read from
 * @param bitStart First bit position to read (0-7)
 * @param length Number of bits to read (not more than 8)
 * @param data Container for right-aligned value (i.e. '101' read from any bitStart position will equal 0x05)
 * @return Status of read operation (count read bytes or BUS_TRANSFER_ERROR)
 */
int32_t I2CBus::ReadBits(uint8_t reg, uint8_t bitStart, uint8_t length, std::byte* data) {
    // 01101001 read byte
    // 76543210 bit numbers
    // xxx   args: bit_start=4, length=3
    // 010   masked
    // -> 010 shifted
    std::byte readByte{0};
    auto countRead = ReadByte(reg, &readByte);
    if (countRead != -1 && countRead != 0) {
        auto mask = static_cast<std::byte>(((1 << length) - 1) << (bitStart - length + 1));
        readByte &= mask;
        readByte >>= (bitStart - length + 1);
        *data = readByte;
    }

    return countRead;
}

/**
 * Read single byte from an 8-bit device register.
 * @param regAddr Register regAddr to read from
 * @param data Container for byte value read from device
 * @return Status of read operation (count read bytes(1) or BUS_TRANSFER_ERROR)
 */
int32_t I2CBus::ReadByte(uint8_t reg, std::byte* data) {
    int32_t ret = -1;
    const size_t kByteSize = 1;
    _buffer[0] = std::byte{reg};

    auto countTxRx = WriteRead(_buffer, _buffer, kByteSize, kByteSize);
    if (countTxRx.first != -1 && countTxRx.second != -1 && countTxRx.second == kByteSize) {
        *data = _buffer[0];
        ret = countTxRx.second;
    }

    return ret;
}

/**
 * Read single word from a 16-bit device register.
 * @param regAddr Register regAddr to read from
 * @param data Container for word value read from device
 * @return Status of read operation (count read bytes or BUS_TRANSFER_ERROR)
 */
int32_t I2CBus::ReadWord(uint8_t reg, uint16_t& data) {
    int32_t ret = -1;
    const size_t kWordSize = 2;
    _buffer[0] = std::byte{reg};

    auto countTxRx = WriteRead(_buffer, _buffer, 1, kWordSize);
    if (countTxRx.first != -1 && countTxRx.second != -1 && countTxRx.second == kWordSize) {
        data = MergeTwoByteInUint16(_buffer[0], _buffer[1]);
        ret = countTxRx.second;
    }

    return ret;
}

/**
 * Read multiple bytes from an 8-bit device register.
 * @param regAddr First register regAddr to read from
 * @param length Number of bytes to read
 * @param data Buffer to store read data in
 * @return Status of read operation (count read bytes or BUS_TRANSFER_ERROR)
 */
int32_t I2CBus::ReadBytes(uint8_t reg, uint8_t length, std::byte* data) {
    int32_t ret = -1;
    _buffer[0] = std::byte{reg};

    auto countTxRx = WriteRead(_buffer, _buffer, 1, length);
    if (countTxRx.first != -1 && countTxRx.second != -1 && countTxRx.second == length) {
        memcpy(data, _buffer, length);
        ret = countTxRx.second;
    }

    return ret;
}

/**
 * Write a single bit in an 8-bit device register.
 * @param regAddr Register regAddr to write to
 * @param bitNum Bit position to write (0-7)
 * @param value New bit value to write
 * @return Status of operation (count write bytes or BUS_TRANSFER_ERROR)
 */
int32_t I2CBus::WriteBit(uint8_t reg, uint8_t bitNum, std::byte data) {
    int32_t ret = -1;
    std::byte readByte{0};
    auto countRead = ReadByte(reg, &readByte);
    if (countRead != -1 && countRead != 0) {
        readByte = (std::to_integer<uint8_t>(data) != 0) ? (readByte | (std::byte{1} << bitNum))
                                                         : (readByte & ~(std::byte{1} << bitNum));

        ret = WriteByte(reg, readByte);
    }

    return ret;
}

/**
 * Write multiple bits in an 8-bit device register.
 * @param regAddr Register regAddr to write to
 * @param bitStart First bit position to write (0-7)
 * @param length Number of bits to write (not more than 8)
 * @param data Right-aligned value to write
 * @return Status of operation (count write bytes or BUS_TRANSFER_ERROR)
 */
int32_t I2CBus::WriteBits(uint8_t reg, uint8_t bitStart, uint8_t length, std::byte data) {
    // 010 value to write
    // 76543210 bit numbers
    // xxx   args: bit_start=4, length=3
    // 00011100 mask byte
    // 10101111 original value (sample)
    // 10100011 original & ~mask
    // 10101011 masked | value
    int32_t ret = -1;
    std::byte read_byte{0};
    auto countRead = ReadByte(reg, &read_byte);

    if (countRead != -1 && countRead != 0) {
        auto mask = static_cast<std::byte>(((1 << length) - 1) << (bitStart - length + 1));
        data <<= (bitStart - length + 1); // shift data into correct position
        data &= mask;                     // zero all non-important bits in data
        read_byte &= ~std::byte{mask};    // zero all important bits in existing byte
        read_byte |= data;                // combine data with existing byte

        ret = WriteByte(reg, read_byte);
    }

    return ret;
}

/**
 * Write single byte to an 8-bit device register.
 * @param regAddr Register address to write to
 * @param data New byte value to write
 * @return Status of operation (count write bytes or BUS_TRANSFER_ERROR)
 */
int32_t I2CBus::WriteByte(uint8_t reg, std::byte data) {
    _buffer[0] = std::byte{reg};
    _buffer[1] = data;
    return Write(_buffer, 2);
}

/**
 * Write single word to a 16-bit device register.
 * @param regAddr Register address to write to
 * @param data New word value to write
 * @return Status of operation (count write bytes or BUS_TRANSFER_ERROR)
 */
int32_t I2CBus::WriteWord(uint8_t reg, uint16_t data) {
    _buffer[0] = std::byte{reg};
    memcpy(&_buffer[1], &data, 2);
    return Write(_buffer, 3);
}

/**
 * Write multiple bytes to an 8-bit device register.
 * @param regAddr First register address to write to
 * @param length Number of bytes to write
 * @param data Buffer to copy new data from
 * @return Status of operation (count write bytes or BUS_TRANSFER_ERROR)
 */
int32_t I2CBus::WriteBytes(uint8_t reg, uint8_t length, std::byte* data) {
    _buffer[0] = std::byte{reg};
    memcpy(_buffer + 1, data, length);
    return Write(_buffer, length + 1);
}

/*!
 * Reinitialization bus
 * @return initialization status
 */
bool I2CBus::ReInit() {
    return _pimpl->ReInit();
}
