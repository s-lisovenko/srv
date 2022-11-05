#include "I2CPwmMultiplexer.h"
#include <iostream>

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
    pwm.setPWMFreq(2000);
    bool lock = false;
    for (uint16_t i = 0; i < 10; i++) {
        pwm.setPWM(0, 0, lock ? 1000 : 2000);
        lock = !lock;
    }


    return 0;
}