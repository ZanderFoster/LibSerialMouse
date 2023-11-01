#pragma once
#include <string>
#include <iostream>
#include <libserialport.h>
#include <vector>

const uint16_t VENDOR_IDS[] = {0x1a86, NULL};
const uint16_t PRODUCT_IDS[] = {0x5523, NULL};

class SerialMouse
{
private:

    // TODO - Implement datalogging for machine learning purposes
    // void parseMovementData(std::string dataBuffer);
    
    // Parse incoming messages relating to the input keys
    void parseModifierData(std::string dataBuffer);
    
    // Read a single line from the serial port
    void ReadLine();

    // Read all data from the serial port and parse it
    void ReadData();

    // Search for a valid SerialMouse device and return the address -- returns null if no device is found
    char* FindDevice(){
        /* A pointer to a null-terminated array of pointers to
        * struct sp_port, which will contain the ports found.*/
        struct sp_port **port_list;

        /* Call sp_list_ports() to get the ports. The port_list
        * pointer will be updated to refer to the array created. */
        enum sp_return result = sp_list_ports(&port_list);

        if (result != SP_OK) {
            std::cout << ("sp_list_ports() failed!\n");
            return NULL;
        }

        /* Iterate through the ports. When port_list[i] is NULL
        * this indicates the end of the list. */
        char *portName;
        int i;
        for (i = 0; port_list[i] != NULL; i++) {
            struct sp_port *port = port_list[i];

            /* Get the name of the port. */
            char *port_name = sp_get_port_name(port);
            int usb_vid, usb_pid;
            sp_get_port_usb_vid_pid(port, &usb_vid, &usb_pid);

            /* Check against the array of valid VID & PID's */            
            for (int j = 0; VENDOR_IDS[j] != NULL; j++) {
                if (usb_vid == VENDOR_IDS[j] && usb_pid == PRODUCT_IDS[j]) {
                    std::cout << "Found valid port:     " << port_name << std::endl;
                    portName = port_name;
                }
            }
        }

        /* Free the array created by sp_list_ports(). */
        sp_free_port_list(port_list);

        /* Note that this will also free all the sp_port structures
        * it points to. If you want to keep one of them (e.g. to
        * use that port in the rest of your program), take a copy
        * of it first using sp_copy_port(). */

        return NULL;
    }

public:
    // Input keys from the physical mouse
    bool prevButton;
    bool nextButton;
    bool leftButton;
    bool rightButton;
    bool middleButton;

    // Try to connect to a valid SerialMouse device
    bool Connect()
    {
        std::cout << "Connecting to SerialMouse device..." << std::endl;
        FindDevice();
        
        return false;
    }
    
    // Disconnect from the serial port
    void Disconnect();

    // Send a move command specifying the x and y coordinates
    void Move(int x = 0, int y = 0);

    // Send a click event that has a humanized click duration
    void Fire();
};

extern SerialMouse cSerialMouse;