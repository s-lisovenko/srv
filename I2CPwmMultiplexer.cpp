#include "I2CPwmMultiplexer.h"
#include <cstddef>
#include <iostream>
#include <thread>

#include "I2cBus.h"


namespace {
// REGISTER ADDRESSES
#define PCA9685_MODE1 0x00      /**< Mode Register 1 */
#define PCA9685_MODE2 0x01      /**< Mode Register 2 */
#define PCA9685_SUBADR1 0x02    /**< I2C-bus subaddress 1 */
#define PCA9685_SUBADR2 0x03    /**< I2C-bus subaddress 2 */
#define PCA9685_SUBADR3 0x04    /**< I2C-bus subaddress 3 */
#define PCA9685_ALLCALLADR 0x05 /**< LED All Call I2C-bus address */
#define PCA9685_LED0_ON_L 0x06  /**< LED0 on tick, low byte*/
#define PCA9685_LED0_ON_H 0x07  /**< LED0 on tick, high byte*/
#define PCA9685_LED0_OFF_L 0x08 /**< LED0 off tick, low byte */
#define PCA9685_LED0_OFF_H 0x09 /**< LED0 off tick, high byte */
// etc all 16:  LED15_OFF_H 0x45
#define PCA9685_ALLLED_ON_L 0xFA  /**< load all the LEDn_ON registers, low */
#define PCA9685_ALLLED_ON_H 0xFB  /**< load all the LEDn_ON registers, high */
#define PCA9685_ALLLED_OFF_L 0xFC /**< load all the LEDn_OFF registers, low */
#define PCA9685_ALLLED_OFF_H 0xFD /**< load all the LEDn_OFF registers,high */
#define PCA9685_PRESCALE 0xFE     /**< Prescaler for PWM output frequency */
#define PCA9685_TESTMODE 0xFF     /**< defines the test mode to be entered */

// MODE1 bits
#define MODE1_ALLCAL 0x01  /**< respond to LED All Call I2C-bus address */
#define MODE1_SUB3 0x02    /**< respond to I2C-bus subaddress 3 */
#define MODE1_SUB2 0x04    /**< respond to I2C-bus subaddress 2 */
#define MODE1_SUB1 0x08    /**< respond to I2C-bus subaddress 1 */
#define MODE1_SLEEP 0x10   /**< Low power mode. Oscillator off */
#define MODE1_AI 0x20      /**< Auto-Increment enabled */
#define MODE1_EXTCLK 0x40  /**< Use EXTCLK pin clock */
#define MODE1_RESTART 0x80 /**< Restart enabled */
// MODE2 bits
#define MODE2_OUTNE_0 0x01 /**< Active LOW output enable input */
#define MODE2_OUTNE_1 0x02 /**< Active LOW output enable input - high impedience */
#define MODE2_OUTDRV 0x04  /**< totem pole structure vs open-drain */
#define MODE2_OCH 0x08     /**< Outputs change on ACK vs STOP */
#define MODE2_INVRT 0x10   /**< Output logic state inverted */

#define PCA9685_I2C_ADDRESS 0x40      /**< Default PCA9685 I2C Slave Address */
#define FREQUENCY_OSCILLATOR 25000000 /**< Int. osc. frequency in datasheet */

#define PCA9685_PRESCALE_MIN 3   /**< minimum prescale value */
#define PCA9685_PRESCALE_MAX 255 /**< maximum prescale value */

} // namespace

I2CPwmMultiplexer::I2CPwmMultiplexer() {
    _bus = std::make_unique<I2CBus>(1, 0x40);
}

I2CPwmMultiplexer::~I2CPwmMultiplexer() = default;

bool I2CPwmMultiplexer::isInit() const {
    return _bus->IsOpen();
}

void I2CPwmMultiplexer::begin(uint8_t prescale) {
    if (prescale) {
        setExtClk(prescale);
    } else {
        // set a default frequency
        setPWMFreq(1000);
    }
    // set the default internal frequency
    setOscillatorFrequency(FREQUENCY_OSCILLATOR);
}

void I2CPwmMultiplexer::reset() {
    std::ignore = _bus->WriteByte(PCA9685_MODE1, static_cast<std::byte>(MODE1_RESTART));
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void I2CPwmMultiplexer::sleep() {
    std::byte data{0};
    std::ignore = _bus->ReadByte(PCA9685_MODE1, &data);
    auto awake = std::to_integer<uint8_t>(data);
    uint8_t sleep = awake | MODE1_SLEEP; // set sleep bit high
    std::ignore = _bus->WriteByte(PCA9685_MODE1, static_cast<std::byte>(sleep));
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

void I2CPwmMultiplexer::wakeup() {
    std::byte data{0};
    std::ignore = _bus->ReadByte(PCA9685_MODE1, &data);
    auto sleep = std::to_integer<uint8_t>(data);
    uint8_t wakeup = (uint8_t) (sleep & ~MODE1_SLEEP); // set sleep bit low
    std::ignore = _bus->WriteByte(PCA9685_MODE1, static_cast<std::byte>(wakeup));
}

void I2CPwmMultiplexer::setExtClk(uint8_t prescale) {
    std::byte data{0};
    std::ignore = _bus->ReadByte(PCA9685_MODE1, &data);
    auto oldmode = std::to_integer<uint8_t>(data);
    uint8_t newmode = (uint8_t) (oldmode & ~MODE1_RESTART) | MODE1_SLEEP; // sleep
    std::ignore = _bus->WriteByte(PCA9685_MODE1,
                                  static_cast<std::byte>(
                                      newmode)); // go to sleep, turn off internal oscillator

    // This sets both the SLEEP and EXTCLK bits of the MODE1 register to switch to
    // use the external clock.
    std::ignore = _bus->WriteByte(PCA9685_MODE1, static_cast<std::byte>(newmode |= MODE1_EXTCLK));

    std::ignore = _bus->WriteByte(PCA9685_PRESCALE, static_cast<std::byte>(prescale)); // set the prescaler

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    // clear the SLEEP bit to start
    std::ignore = _bus->WriteByte(PCA9685_MODE1,
                                  static_cast<std::byte>((newmode & ~MODE1_SLEEP) | MODE1_RESTART | MODE1_AI));
}

void I2CPwmMultiplexer::setPWMFreq(float freq) {
    // Range output modulation frequency is dependant on oscillator
    if (freq < 1)
        freq = 1;
    if (freq > 3500)
        freq = 3500; // Datasheet limit is 3052=50MHz/(4*4096)

    float prescaleval = static_cast<float>(((_oscillator_freq / (freq * 4096.0)) + 0.5) - 1);
    std::cout << "prescale: " << prescaleval << "\n";
    if (prescaleval < PCA9685_PRESCALE_MIN)
        prescaleval = PCA9685_PRESCALE_MIN;
    if (prescaleval > PCA9685_PRESCALE_MAX)
        prescaleval = PCA9685_PRESCALE_MAX;
    uint8_t prescale = (uint8_t) prescaleval;

    std::byte data{0};
    auto countRead = _bus->ReadByte(PCA9685_MODE1, &data);
    auto oldmode = (uint8_t)(data);

    uint8_t newmode = (uint8_t) ((oldmode & ~MODE1_RESTART) | MODE1_SLEEP);            // sleep
    std::ignore = _bus->WriteByte(PCA9685_MODE1, static_cast<std::byte>(newmode));     // go to sleep
    std::ignore = _bus->WriteByte(PCA9685_PRESCALE, static_cast<std::byte>(prescale)); // set the prescaler
    std::ignore = _bus->WriteByte(PCA9685_MODE1, static_cast<std::byte>(oldmode));
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    // This sets the MODE1 register to turn on auto increment.
    std::ignore = _bus->WriteByte(PCA9685_MODE1, static_cast<std::byte>(oldmode | MODE1_RESTART | MODE1_AI));
}

void I2CPwmMultiplexer::setOutputMode(bool totempole) {
    std::byte data{0};
    std::ignore = _bus->ReadByte(PCA9685_MODE2, &data);
    auto oldmode = std::to_integer<uint8_t>(data);
    uint8_t newmode;
    if (totempole) {
        newmode = (uint8_t) (oldmode | MODE2_OUTDRV);
    } else {
        newmode = (uint8_t) (oldmode & ~MODE2_OUTDRV);
    }
    std::ignore = _bus->WriteByte(PCA9685_MODE2, static_cast<std::byte>(newmode));
}

uint8_t I2CPwmMultiplexer::getPWM(uint8_t num) {
    uint16_t data;
    std::ignore = _bus->ReadWord((uint8_t) (PCA9685_LED0_ON_L + 4 * num), data);
    return data;
}

uint8_t I2CPwmMultiplexer::setPWM(uint8_t num, uint16_t on, uint16_t off) {
    auto addr = (uint8_t) (PCA9685_LED0_ON_L + 4 * num);
    std::byte data[4] = {static_cast<std::byte>(on),
                         static_cast<std::byte>(on >> 8),
                         static_cast<std::byte>(off),
                         static_cast<std::byte>(off >> 8)};
    std::ignore = _bus->WriteBytes(addr, 4, data);
    return 1;
}

void I2CPwmMultiplexer::setPin(uint8_t num, uint16_t val, bool invert) {
    // Clamp value between 0 and 4095 inclusive.
    val = std::min(val, (uint16_t) 4095);
    if (invert) {
        if (val == 0) {
            // Special value for signal fully on.
            setPWM(num, 4096, 0);
        } else if (val == 4095) {
            // Special value for signal fully off.
            setPWM(num, 0, 4096);
        } else {
            setPWM(num, 0, 4095 - val);
        }
    } else {
        if (val == 4095) {
            // Special value for signal fully on.
            setPWM(num, 4096, 0);
        } else if (val == 0) {
            // Special value for signal fully off.
            setPWM(num, 0, 4096);
        } else {
            setPWM(num, 0, val);
        }
    }
}

uint8_t I2CPwmMultiplexer::readPrescale(void) {
    std::byte data{0};
    std::ignore = _bus->ReadByte(PCA9685_PRESCALE, &data);
    return std::to_integer<uint8_t>(data);
}

void I2CPwmMultiplexer::writeMicroseconds(uint8_t num, uint16_t Microseconds) {
    double pulse = Microseconds;
    double pulselength;
    pulselength = 1000000; // 1,000,000 us per second

    // Read prescale
    uint16_t prescale = readPrescale();

    // Calculate the pulse for PWM based on Equation 1 from the datasheet section
    // 7.3.5
    prescale += 1;
    pulselength *= prescale;
    pulselength /= _oscillator_freq;

    pulse /= pulselength;

    setPWM(num, 0, static_cast<uint16_t>(pulse));
}

void I2CPwmMultiplexer::setOscillatorFrequency(uint32_t freq) {
    _oscillator_freq = freq;
}

uint32_t I2CPwmMultiplexer::getOscillatorFrequency() {
    return _oscillator_freq;
}
