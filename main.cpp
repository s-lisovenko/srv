#include "I2CPwmMultiplexer.h"
#include <iostream>
#include <thread>

#define SERVOMIN 150 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX 600 // This is the 'maximum' pulse length count (out of 4096)
#define USMIN 600    // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
#define USMAX 2400   // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600
#define SERVO_FREQ 50// Analog servos run at ~50 Hz updates

int main()
{
    auto &pwm = I2CPwmMultiplexer::instance();
    if (!pwm.isInit()) {
        std::cerr << "I2C not inited!\n";
        return 1;
    }

    pwm.begin();
    pwm.setOscillatorFrequency(27000000);
    pwm.setPWMFreq(SERVO_FREQ);// Analog servos run at ~50 Hz updates
    uint8_t servonum = 0;

    for (int i = 0; i < 10; ++i) {
        for (uint8_t pin = 0; pin < 16; pin++) {
            pwm.setPWM(pin, 4096, 0);// turns pin fully on
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            pwm.setPWM(pin, 0, 4096);// turns pin fully off
        }
    }

    return 0;
}
