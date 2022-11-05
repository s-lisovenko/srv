#include "I2CDevImpl.h"

#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <mutex>
#include <thread>

std::shared_ptr<I2CDeviceImpl> I2CDeviceImpl::Instance(uint32_t busNumber, uint32_t mode) {
    // static variable
    static std::mutex i2cBusMutex;
    static std::shared_ptr<I2CDeviceImpl> i2cDevice0(nullptr);
    static std::shared_ptr<I2CDeviceImpl> i2cDevice1(nullptr);
    static const uint32_t bus0 = 0;
    static const uint32_t bus1 = 1;

    if (busNumber == bus0) {
        if (i2cDevice0 == nullptr) {
            std::lock_guard<std::mutex> lock(i2cBusMutex);
            if (i2cDevice0 == nullptr) {
                i2cDevice0 = std::shared_ptr<I2CDeviceImpl>(new I2CDeviceImpl(busNumber, mode));
            }
        }
        return i2cDevice0;
    }
    if (busNumber == bus1) {
        if (i2cDevice1 == nullptr) {
            std::lock_guard<std::mutex> lock(i2cBusMutex);
            if (i2cDevice1 == nullptr) {
                i2cDevice1 = std::shared_ptr<I2CDeviceImpl>(new I2CDeviceImpl(busNumber, mode));
            }
        }
        return i2cDevice1;
    }
    fprintf(stderr, "Invalid i2c bus number. Use 1 or 0. Error message: %s", strerror(errno));

    return nullptr;
}

bool I2CDeviceImpl::IsOpen() const {
    return _descriptor > _kBadFileDescriptor;
}

std::pair<int32_t, int32_t> I2CDeviceImpl::WriteRead(std::byte* txBuf,
                                                     std::byte* rxBuf,
                                                     size_t bytesToTransfer,
                                                     size_t bytesToReceive) const {
    auto countReceived = -1;
    auto countTransferred = Write(txBuf, bytesToTransfer);
    if (countTransferred != -1) {
        countReceived = Read(rxBuf, bytesToReceive);
    }
    return {countTransferred, countReceived};
}

int32_t I2CDeviceImpl::Write(std::byte* txBuf, size_t bytesToTransfer) const {
    const auto countBytesWrite = write(_descriptor, txBuf, bytesToTransfer);
    if (countBytesWrite != static_cast<int>(bytesToTransfer)) {
        return -1;
    }
    return static_cast<int32_t>(countBytesWrite);
}

int32_t I2CDeviceImpl::Read(std::byte* rxBuf, size_t bytesToReceive) const {
    const auto countBytesRead = read(_descriptor, rxBuf, bytesToReceive);
    if (countBytesRead != static_cast<int>(bytesToReceive)) {
        return -1;
    }
    return static_cast<int32_t>(countBytesRead);
}

I2CDeviceImpl::~I2CDeviceImpl() {
    Close();
}

const std::error_code& I2CDeviceImpl::SetMode() {
    _errorCode.clear();

    if (_address == _kBadDeviceAddress) {
        _errorCode.assign(errno, std::generic_category());
        fprintf(stderr,
                "Failed change mode because. Address is bad. Error message: %s",
                _errorCode.message().c_str());
        return _errorCode;
    }

    if (!IsOpen()) {
        _errorCode.assign(errno, std::generic_category());
        fprintf(stderr,
                "Failed change mode because file descriptor is closed. Error message: %s",
                _errorCode.message().c_str());
        return _errorCode;
    }

    auto result = ioctl(_descriptor, _mode, _address);
    if (result < 0) {
        _errorCode.assign(errno, std::generic_category());
        fprintf(stderr,
                "Failed to acquire bus access and/or talk to slave. Error message: %s",
                _errorCode.message().c_str());
        return _errorCode;
    }

    return _errorCode;
}

void I2CDeviceImpl::SetCommunicationMode(uint32_t mode) {
    if (mode != _mode) {
        _mode = mode;
        std::ignore = SetMode();
    }
}

void I2CDeviceImpl::SetCommunicationAddress(int32_t address) {
    if (address != _address) {
        _address = address;
        std::ignore = SetMode();
    }
}

I2CDeviceImpl::I2CDeviceImpl(uint32_t busNumber, uint32_t mode)
    : _busNumber(busNumber)
    , _mode(mode) {
    Open(_busNumber);
}

bool I2CDeviceImpl::Open(uint32_t busNumber) {
    char devicePath[_kMaxFilenamePath] = {0};
    int bytesCopied = snprintf(devicePath, _kMaxFilenamePath, "%s%u", _kDevicePath, busNumber);

    if (bytesCopied > 0) {
        _descriptor = open(devicePath, O_RDWR);
    }

    return _descriptor != _kBadFileDescriptor;
}

void I2CDeviceImpl::Close() {
    close(_descriptor);
    _descriptor = _kBadFileDescriptor;
}

bool I2CDeviceImpl::ReInit() {
    Close();
    return Open(_busNumber);
}
