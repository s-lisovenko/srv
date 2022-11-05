cmake_minimum_required(VERSION 3.23)
project(servo_test)

set(CMAKE_CXX_STANDARD 17)

add_executable(servo_test main.cpp I2CPwmMultiplexer.cpp I2CDevImpl.cpp I2cBus.cpp)
