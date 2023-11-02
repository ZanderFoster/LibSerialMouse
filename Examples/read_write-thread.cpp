#include <iostream>
#include <chrono>
#include "SerialMouse.h"

SerialMouse cSerialMouse;

int main() {
    if ( !cSerialMouse.Connect()) {
        std::cout << "Failed to connect to serial mouse exiting" << std::endl;
        return 1;
    }

    std::cout << "Starting SerialMouse read thread" << std::endl;
    cSerialMouse.StartThread();

    while (d_SerialMouse.isSerialMouseConnected){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        cSerialMouse.Move(1, 1);
    }
}