#include "I2CPwmMultiplexer.h"
#include <iostream>
#include <unistd.h>

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cout << "Please pass 2 parameters. Programm <servo_channel> <servo_frequency>";
        return 1;
    }

    int channel = atoi(argv[1]);
    int freq = atoi(argv[2]);

    auto &pwm = I2CPwmMultiplexer::instance();
    if (!pwm.isInit()) {
        std::cerr << "I2C not inited!\n";
        return 1;
    }
    pwm.setPwmFreq(freq);

    int pwmUsec;
    while (std::cin >> pwmUsec) {
        pwm.setPwmMs(channel, (double)pwmUsec / 1000);

        if (pwmUsec < 0) {
            return 0;
        }
    }
    return 0;
}
