#include "I2CPwmMultiplexer.h"
#include <iostream>
#include <thread>

#define SERVOMIN 1000 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX 2000 // This is the 'maximum' pulse length count (out of 4096)
#define SERVO_FREQ 60// Analog servos run at ~60 Hz updates

int main()
{
    int servonum = 0;
    auto &pwm = I2CPwmMultiplexer::instance();
    if (!pwm.isInit()) {
        std::cerr << "I2C not inited!\n";
        return 1;
    }

    pwm.begin();
    pwm.setPWMFreq(SERVO_FREQ);// Analog servos run at ~50 Hz updates

    for (int i = 0; i < 10; ++i) {
        for (uint16_t pulselen = SERVOMIN; pulselen < SERVOMAX; pulselen++) {
            pwm.setPWM(servonum, 0, pulselen);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        for (uint16_t pulselen = SERVOMAX; pulselen > SERVOMIN; pulselen--) {
            pwm.setPWM(servonum, 0, pulselen);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        std::cout << "pwm: " << pwm.getPWM(0) << "\n";
    }
    return 0;
}
