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
        // Drive each servo one at a time using setPWM()

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


        servonum++;
        if (servonum > 7) servonum = 0;// Testing the first 8 servo channels
    }

    return 0;
}
