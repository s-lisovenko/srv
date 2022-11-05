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
    static I2CPwmMultiplexer &instance()
    {
        static I2CPwmMultiplexer mixer;
        return mixer;
    }

    [[nodiscard]] bool isInit() const;

    /*!
     *  @brief  Sets the PWM frequency for the entire chip, up to ~1.6 KHz
     *  @param  freq Floating point frequency that we will attempt to match
     */
    void setPwmFreq(double freqHz);

    /*!
     *  @brief  Sets the PWM output of one of the PCA9685 pins
     *  @param  channel One of the PWM output pins, from 0 to 15
     *  @param  on At what point in the 4096-part cycle to turn the PWM output ON
     *  @param  off At what point in the 4096-part cycle to turn the PWM output OFF
     */
    void setPwm(int channel, uint16_t on, uint16_t off);

    /*!
     * Sets the PWM output of all of the PCA9685 pins
     * @param  on At what point in the 4096-part cycle to turn the PWM output ON
     * @param  off At what point in the 4096-part cycle to turn the PWM output OFF
     */
    void setAllPwm(uint16_t on, uint16_t off);

    /*!
     *  @brief  Sets the PWM output of one of the PCA9685 pins based on the input
     *  microseconds, output is not precise
     *  @param  channel One of the PWM output pins, from 0 to 15
     *  @param  ms The number of Microseconds to turn the PWM output ON
     */
    void setPwmMs(int channel, double ms);

private:
    I2CPwmMultiplexer();

private:
    // Default frequency pulled from PCA9685 datasheet.
    double _frequency{200.0};
    std::unique_ptr<I2CBus> _bus{nullptr};
};

#endif// I2CPWMMULTIPLEXER_H
