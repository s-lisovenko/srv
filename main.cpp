#include "I2CPwmMultiplexer.h"
#include <iostream>
#include <thread>

#define SERVOMIN 100 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX 2200// This is the 'maximum' pulse length count (out of 4096)
#define SERVO_FREQ 60// Analog servos run at ~60 Hz updates
#define USMIN 600    // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
#define USMAX 2400   // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600

auto &pwm = I2CPwmMultiplexer::instance();

int main()
{
    if (!pwm.isInit()) {
        std::cerr << "I2C not inited!\n";
        return 1;
    }
    pwm.begin();
    pwm.setPWMFreq(SERVO_FREQ);// Analog servos run at ~50 Hz updates
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    uint8_t servonum = 0;
    while (true) {
        for (uint16_t pulselen = SERVOMIN; pulselen < SERVOMAX; pulselen++) {
            pwm.setPWM(servonum, 0, pulselen);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        for (uint16_t pulselen = SERVOMAX; pulselen > SERVOMIN; pulselen--) {
            pwm.setPWM(servonum, 0, pulselen);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Drive each servo one at a time using writeMicroseconds(), it's not precise due to calculation rounding!
        // The writeMicroseconds() function is used to mimic the Arduino Servo library writeMicroseconds() behavior.
        for (uint16_t microsec = USMIN; microsec < USMAX; microsec++) {
            pwm.writeMicroseconds(servonum, microsec);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        for (uint16_t microsec = USMAX; microsec > USMIN; microsec--) {
            pwm.writeMicroseconds(servonum, microsec);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        std::cout << "pwm for servo" << servonum << ": " << pwm.getPWM(servonum);
        servonum++;
        if (servonum > 7) servonum = 0;// Testing the first 8 servo channels
    }
    return 0;
}
