#include <iostream>
#include "SerialMouse.h"

int main() {
    SerialMouse mouse;
    if(mouse.Connect()){
        std::cout << "Connected to SerialMouse device!" << std::endl;
    } else {
        std::cout << "Failed to connect to SerialMouse device!" << std::endl;
    }

    return 0;
}