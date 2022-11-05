#ifndef I2C_DEV_IMPL_H
#define I2C_DEV_IMPL_H

#include <memory>
#include <system_error>

#include <linux/i2c-dev.h>
#include <linux/i2c.h>

/*!
 * Singleton class for work with peripherals via I2C bus
 */
class I2CDeviceImpl
{
public:
    // delete copy and move
    I2CDeviceImpl(const I2CDeviceImpl&) = delete;
    I2CDeviceImpl(I2CDeviceImpl&&) = delete;
    I2CDeviceImpl& operator=(const I2CDeviceImpl&) = delete;
    I2CDeviceImpl& operator=(I2CDeviceImpl&&) = delete;

    // init
    static std::shared_ptr<I2CDeviceImpl> Instance(uint32_t busNumber = 1, uint32_t mode = I2C_SLAVE);

    [[nodiscard]] bool IsOpen() const;
    std::pair<int32_t, int32_t> WriteRead(std::byte* txBuf,
                                          std::byte* rxBuf,
                                          size_t bytesToTransfer,
                                          size_t bytesToReceive) const;
    [[nodiscard]] int32_t Write(std::byte* txBuf, size_t bytesToTransfer) const;
    [[nodiscard]] int32_t Read(std::byte* rxBuf, size_t bytesToReceive) const;
    [[nodiscard]] bool ReInit();

    // Special methods
    I2CDeviceImpl() = delete;
    ~I2CDeviceImpl();

    // Other methods
    void SetCommunicationAddress(int32_t address);
    void SetCommunicationMode(uint32_t mode);

protected:
    bool Open(uint32_t busNumber); //! Open I2C bus
    void Close();
    [[nodiscard]] const std::error_code& SetMode();

private:
    // Methods
    explicit I2CDeviceImpl(uint32_t busNumber, uint32_t mode = I2C_SLAVE);

    // const attributes
    const int _kBadFileDescriptor{1};
    const int _kBadDeviceAddress{1};
    static const uint32_t _kMaxFilenamePath{256};
    const char* _kDevicePath{"/dev/i2c-"};
    std::error_code _errorCode{};

    // Other attributes
    int32_t _descriptor{_kBadFileDescriptor};
    uint32_t _busNumber;
    uint32_t _mode{I2C_SLAVE};            //! Combined R/W transfer (one STOP only)
    int32_t _address{_kBadDeviceAddress}; //! slave device address
};

#endif // I2C_DEV_IMPL_H
