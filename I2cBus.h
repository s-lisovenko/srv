#ifndef I2C_BUS_H
#define I2C_BUS_H

#include <memory>

class I2CDeviceImpl;

class I2CBus
{
public:
    I2CBus() = delete;
    explicit I2CBus(uint32_t busNumber, int32_t deviceAddress);
    ~I2CBus() = default;

    // delete copy and move
    I2CBus(const I2CBus&) = delete;
    I2CBus(I2CBus&&) = delete;
    I2CBus& operator=(const I2CBus&) = delete;
    I2CBus& operator=(I2CBus&&) = delete;

    [[nodiscard]] bool IsOpen() const;
    [[nodiscard]] std::pair<int32_t, int32_t> WriteRead(std::byte* txBuf,
                                                        std::byte* rxBuf,
                                                        size_t bytesToTransfer,
                                                        size_t bytesToReceive);
    int32_t Write(std::byte* txBuf, size_t bytesToTransfer);
    int32_t Read(std::byte* rxBuf, size_t bytesToTransfer);
    bool ReInit();

    [[nodiscard]] int32_t ReadBit(uint8_t reg, uint8_t bitNum, std::byte* data);
    [[nodiscard]] int32_t ReadBits(uint8_t reg, uint8_t bitStart, uint8_t length, std::byte* data);
    [[nodiscard]] int32_t ReadByte(uint8_t reg, std::byte* data);
    [[nodiscard]] int32_t ReadWord(uint8_t reg, uint16_t& data);
    [[nodiscard]] int32_t ReadBytes(uint8_t reg, uint8_t length, std::byte* data);

    [[nodiscard]] int32_t WriteBit(uint8_t reg, uint8_t bitNum, std::byte data);
    [[nodiscard]] int32_t WriteBits(uint8_t reg, uint8_t bitStart, uint8_t length, std::byte data);
    [[nodiscard]] int32_t WriteByte(uint8_t reg, std::byte data);
    [[nodiscard]] int32_t WriteWord(uint8_t reg, uint16_t data);
    [[nodiscard]] int32_t WriteBytes(uint8_t reg, uint8_t length, std::byte* data);

    // set
    void ChangeCommunicationMode(uint32_t mode);
    void setAddress(int32_t address) { _deviceAddress = address; }

    [[nodiscard]] int32_t device_address() const { return _deviceAddress; }

protected:
    void ChangeCommunicationAddress();

private:
    std::byte _buffer[128]{};
    int32_t _deviceAddress;
    std::shared_ptr<I2CDeviceImpl> _pimpl;
};

#endif // I2C_BUS_H
