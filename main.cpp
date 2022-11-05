#include "I2CPwmMultiplexer.h"
#include <iostream>
#include <thread>

#define SERVOMIN 1000// This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX 2000// This is the 'maximum' pulse length count (out of 4096)
#define SERVO_FREQ 60// Analog servos run at ~60 Hz updates

auto &pwm = I2CPwmMultiplexer::instance();

void setServoPulse(uint8_t n, double pulse)
{
    double pulselength;

    pulselength = 1000000;    // 1,000,000 us per second
    pulselength /= SERVO_FREQ;// Analog servos run at ~60 Hz updates
    pulselength /= 4096;      // 12 bits of resolution
    pulse *= 1000000;         // convert input seconds to us
    pulse /= pulselength;
    pwm.setPWM(n, 0, pulse);
}


int main()
{
    if (!pwm.isInit()) {
        std::cerr << "I2C not inited!\n";
        return 1;
    }
    pwm.begin();
    pwm.setPWMFreq(SERVO_FREQ);// Analog servos run at ~50 Hz updates

    for (int i = 0; i < 10; ++i) {
        setServoPulse(0,0.4);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        setServoPulse(0,1.4);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        setServoPulse(0,2.4);
    }
    return 0;
}
