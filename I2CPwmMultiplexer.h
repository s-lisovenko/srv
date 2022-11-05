#ifndef I2CPWMMULTIPLEXER_H
#define I2CPWMMULTIPLEXER_H

#include <memory>

class I2CBus;

/**
 * pca9685 multiplexer
 */
class I2CPwmMultiplexer
{
public:
    ~I2CPwmMultiplexer();

    // Instance singleton
    static I2CPwmMultiplexer& instance() {
        static I2CPwmMultiplexer mixer;
        return mixer;
    }

    [[nodiscard]] bool isInit() const;

    /*!
     *  @brief  Setups the I2C interface and hardware
     *  @param  prescale
     *          Sets External Clock (Optional)
     */
    void begin(uint8_t prescale = 0);

    /*!
     *  @brief  Sends a reset command to the PCA9685 chip over I2C
     */
    void reset();

    /*!
     *  @brief  Puts board into sleep mode
     */
    void sleep();

    /*!
     *  @brief  Wakes board from sleep
     */
    void wakeup();

    /*!
     *  @brief  Sets EXTCLK pin to use the external clock
     *  @param  prescale
     *          Configures the prescale value to be used by the external clock
     */
    void setExtClk(uint8_t prescale);

    /*!
     *  @brief  Sets the PWM frequency for the entire chip, up to ~1.6 KHz
     *  @param  freq Floating point frequency that we will attempt to match
     */
    void setPWMFreq(float freq);

    /*!
     *  @brief  Sets the output mode of the PCA9685 to either
     *  open drain or push pull / totempole.
     *  Warning: LEDs with integrated zener diodes should
     *  only be driven in open drain mode.
     *  @param  totempole Totempole if true, open drain if false.
     */
    void setOutputMode(bool totempole);

    /*!
     *  @brief  Gets the PWM output of one of the PCA9685 pins
     *  @param  num One of the PWM output pins, from 0 to 15
     *  @return requested PWM output value
     */
    uint8_t getPWM(uint8_t num);

    /*!
     *  @brief  Sets the PWM output of one of the PCA9685 pins
     *  @param  num One of the PWM output pins, from 0 to 15
     *  @param  on At what point in the 4096-part cycle to turn the PWM output ON
     *  @param  off At what point in the 4096-part cycle to turn the PWM output OFF
     *  @return result from endTransmission
     */
    uint8_t setPWM(uint8_t num, uint16_t on, uint16_t off);

    /*!
     *   @brief  Helper to set pin PWM output. Sets pin without having to deal with
     * on/off tick placement and properly handles a zero value as completely off and
     * 4095 as completely on.  Optional invert parameter supports inverting the
     * pulse for sinking to ground.
     *   @param  num One of the PWM output pins, from 0 to 15
     *   @param  val The number of ticks out of 4096 to be active, should be a value
     * from 0 to 4095 inclusive.
     *   @param  invert If true, inverts the output, defaults to 'false'
     */
    void setPin(uint8_t num, uint16_t val, bool invert = false);

    /*!
     *  @brief  Reads set Prescale from PCA9685
     *  @return prescale value
     */
    uint8_t readPrescale();

    /*!
     *  @brief  Sets the PWM output of one of the PCA9685 pins based on the input
     * microseconds, output is not precise
     *  @param  num One of the PWM output pins, from 0 to 15
     *  @param  Microseconds The number of Microseconds to turn the PWM output ON
     */
    void writeMicroseconds(uint8_t num, uint16_t Microseconds);

    /*!
     *  @brief Setter for the internally tracked oscillator used for freq
     * calculations
     *  @param freq The frequency the PCA9685 should use for frequency calculations
     */
    void setOscillatorFrequency(uint32_t freq);

    /*!
     *  @brief  Getter for the internally tracked oscillator used for freq
     * calculations
     *  @returns The frequency the PCA9685 thinks it is running at (it cannot
     * introspect)
     */
    uint32_t getOscillatorFrequency();

private:
    I2CPwmMultiplexer();

private:
    uint32_t _oscillator_freq;
    std::unique_ptr<I2CBus> _bus{nullptr};
};

#endif // I2CPWMMULTIPLEXER_H
