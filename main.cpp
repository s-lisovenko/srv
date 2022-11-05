#include "I2CPwmMultiplexer.h"
#include <iostream>
#include <unistd.h>

int main()
{
    auto &pwm = I2CPwmMultiplexer::instance();
    if (!pwm.isInit()) {
        std::cerr << "I2C not inited!\n";
        return 1;
    }
    pwm.setPwmFreq(60);

    while (true) {
        pwm.setPwm(0, 0, 370);
        usleep(1'000'000);
        pwm.setPwm(0, 0, 415);
        usleep(1'000'000);
        pwm.setPwm(0, 0, 460);
        usleep(1'000'000);
        pwm.setPwm(0, 0, 415);
        usleep(1'000'000);
    }
    return 0;
}
