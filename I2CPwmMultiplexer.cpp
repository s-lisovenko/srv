#include "I2CPwmMultiplexer.h"

#include <cmath>
#include <cstddef>
#include <unistd.h>
#include <iostream>

#include "I2cBus.h"

namespace
{
// Registers/etc:
constexpr uint8_t MODE1 = 0x00;
constexpr uint8_t MODE2 = 0x01;
constexpr uint8_t SUBADR1 = 0x02;
constexpr uint8_t SUBADR2 = 0x03;
constexpr uint8_t SUBADR3 = 0x04;
constexpr uint8_t PRESCALE = 0xFE;
constexpr uint8_t LED0_ON_L = 0x06;
constexpr uint8_t LED0_ON_H = 0x07;
constexpr uint8_t LED0_OFF_L = 0x08;
constexpr uint8_t LED0_OFF_H = 0x09;
constexpr uint8_t ALL_LED_ON_L = 0xFA;
constexpr uint8_t ALL_LED_ON_H = 0xFB;
constexpr uint8_t ALL_LED_OFF_L = 0xFC;
constexpr uint8_t ALL_LED_OFF_H = 0xFD;

// Bits:
constexpr uint8_t RESTART = 0x80;
constexpr uint8_t SLEEP = 0x10;
constexpr uint8_t ALLCALL = 0x01;
constexpr uint8_t INVRT = 0x10;
constexpr uint8_t OUTDRV = 0x04;

}// namespace

I2CPwmMultiplexer::I2CPwmMultiplexer()
{
    _bus = std::make_unique<I2CBus>(1, 0x40);

    setAllPwm(0, 0);
    std::ignore = _bus->WriteByte(MODE2, (std::byte) OUTDRV);
    std::ignore = _bus->WriteByte(MODE1, (std::byte) ALLCALL);
    usleep(5'000);
    std::byte data{0};
    std::ignore = _bus->ReadByte(MODE1, &data);
    auto mode1Val = std::to_integer<int>(data);
    mode1Val &= ~SLEEP;
    std::ignore = _bus->WriteByte(MODE1, static_cast<std::byte>(mode1Val));
    usleep(5'000);
}

I2CPwmMultiplexer::~I2CPwmMultiplexer() = default;

bool I2CPwmMultiplexer::isInit() const
{
    return _bus->IsOpen();
}

void I2CPwmMultiplexer::setPwmFreq(const double freqHz)
{
    _frequency = freqHz;

    auto prescaleval = 2.5e7;//    # 25MHz
    prescaleval /= 4096.0;   //       # 12-bit
    prescaleval /= freqHz;
    prescaleval -= 1.0;

    auto prescale = static_cast<int>(std::round(prescaleval));

    std::byte data{0};
    std::ignore = _bus->ReadByte(MODE1, &data);
    const auto oldMode = std::to_integer<int>(data);
    auto newMode = (oldMode & 0x7F) | SLEEP;

    std::ignore = _bus->WriteByte(MODE1, static_cast<std::byte>(newMode));
    std::ignore = _bus->WriteByte(PRESCALE, static_cast<std::byte>(prescale));
    std::ignore = _bus->WriteByte(MODE1, (std::byte) oldMode);
    usleep(5'000);
    std::ignore = _bus->WriteByte(MODE1, static_cast<std::byte>(oldMode | RESTART));
}

void I2CPwmMultiplexer::setPwm(const int channel, const uint16_t on, const uint16_t off)
{
    std::cout << "Pwm: " << off << "\n";
    const auto channel_offset = 4 * channel;
    std::ignore = _bus->WriteByte(LED0_ON_L + channel_offset, static_cast<std::byte>(on & 0xFF));
    std::ignore = _bus->WriteByte(LED0_ON_H + channel_offset, static_cast<std::byte>(on >> 8));
    std::ignore = _bus->WriteByte(LED0_OFF_L + channel_offset, static_cast<std::byte>(off & 0xFF));
    std::ignore = _bus->WriteByte(LED0_OFF_H + channel_offset, static_cast<std::byte>(off >> 8));
}

void I2CPwmMultiplexer::setAllPwm(const uint16_t on, const uint16_t off)
{
    std::ignore = _bus->WriteByte(ALL_LED_ON_L, static_cast<std::byte>(on & 0xFF));
    std::ignore = _bus->WriteByte(ALL_LED_ON_H, static_cast<std::byte>(on >> 8));
    std::ignore = _bus->WriteByte(ALL_LED_OFF_L, static_cast<std::byte>(off & 0xFF));
    std::ignore = _bus->WriteByte(ALL_LED_OFF_H, static_cast<std::byte>(off >> 8));
}

void I2CPwmMultiplexer::setPwmMs(const int channel, const double ms)
{
    auto period_ms = 1000.0 / _frequency;
    auto bits_per_ms = 4096 / period_ms;
    auto bits = ms * bits_per_ms;
    setPwm(channel, 0, bits);
}
