#pragma once
#include <libserialport.h>
#include <iostream>

#include <sstream>
#include <chrono>
#include <thread>
#include <string>
#include <cstring>
typedef struct SerialMouseInfo {
    bool prevButton = false;
    bool nextButton = false;

    //bool leftButton = false;
    //bool rightButton = false;
    //bool middleButton = false;

    bool isSerialMouseConnected = false;
    struct sp_port *port = NULL;
    char portName[256] = "";
} SerialMouseInfo;

SerialMouseInfo d_SerialMouse;


const uint16_t VENDOR_IDS[] = {0x1a86, NULL};
const uint16_t PRODUCT_IDS[] = {0x5523, NULL};
class SerialMouse
{
private:
    /**
     * @brief Helper function for error handling
     * @param result A good result will return SP_OK
     */
    int check(enum sp_return result)
    {
            /* For this example we'll just exit on any error by calling abort(). */
            char *error_message;
            switch (result) {
            case SP_ERR_ARG:
                    printf("Error: Invalid argument.\n");
                    abort();
            case SP_ERR_FAIL:
                    error_message = sp_last_error_message();
                    printf("Error: Failed: %s\n", error_message);
                    sp_free_error_message(error_message);
                    abort();
            case SP_ERR_SUPP:
                    printf("Error: Not supported.\n");
                    abort();
            case SP_ERR_MEM:
                    printf("Error: Couldn't allocate memory.\n");
                    abort();
            case SP_OK:
            default:
                    return result;
            }
    }
    
    /**
     * @brief Parse incoming modifier commands and update the d_SerialMouse struct.
     * @param dataBuffer The #key:value$ command to parse.
     */
    void ParseModifierData(const std::string& dataBuffer)
    {
        std::istringstream ss(dataBuffer.substr(1));
        int modifierButton;
        ss >> modifierButton;
        ss.str(dataBuffer.substr(3));
        int modifierStatus;
        ss >> modifierStatus;

        switch (modifierButton)
        {
            case 1:
                d_SerialMouse.nextButton = modifierStatus != 0;
                break;
            case 2:
                d_SerialMouse.prevButton = modifierStatus != 0;
                break;
        }
    }
    
    /**
     * @brief Reads a single line of data from the serial port.
     * @return Returns a string
     * @param msTimeout The timeout in milliseconds. 0 = no timeout.
     */
    std::string ReadLine(int msTimeout) 
    {
        if (!d_SerialMouse.port) {
            std::cerr << "[SM] Serial port is not open." << std::endl;
            return "";
        }

        std::string line;
        char next_char = '\0';
        bool startCharFound = false;  // Flag to track if the start character is found
        std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
        bool readComplete = false;  // Flag to track if the line has been fully read

        while (!readComplete) {
            int result = sp_nonblocking_read(d_SerialMouse.port, &next_char, 1);

            if (result > 0) {
                if (next_char == '@' || next_char == '#') {
                    // Found the start character, so set the flag
                    startCharFound = true;
                    line = next_char;  // Start building the line with the start character
                } else if (startCharFound) {
                    // Start character has been found, so add the current character to the line
                    line += next_char;
                    if (next_char == '$') {
                        // Found the end character, so the line is complete
                        readComplete = true;
                    }
                }
            }

            // Calculate the time elapsed since the start
            auto current_time = std::chrono::high_resolution_clock::now();
            auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);

            if (msTimeout > 0 && elapsed_time.count() > msTimeout) {
                std::cerr << "[SM] Read timeout." << std::endl;
                return "";
            }
        }

        return line;
    }

    /**
     * @brief Extracts commands and sends the data to the apropriate handlers.
     */
    void ReadData()
    {
        // Verify the port is open
        if (!d_SerialMouse.port) {
            std::cout << "[SM] ReadData -> Mouse Connection Error." << std::endl;
        }

        // Verify that there is data available to read
        if (!sp_input_waiting(d_SerialMouse.port)) {
            return;
        }

        std::string result = ReadLine(0);
        
        if(result.empty()){
            return;
        }

        // Check for the command identifier character
        switch (result[0])
        {
            case '@':
                // TODO ParseMouseData(result);
                break;
            case '#':
                ParseModifierData(result);
                break;
            default:
                std::cout << "[SM] Unknown command: " << result << std::endl;
                break;
        }

    }


    /** 
     * @brief Search for a valid SerialMouse device and return the address
     * @return Returns the port to the class port variable
     * @return or Returns NULL if no valid device is found
     */
    void FindDevice(){
        /* A pointer to a null-terminated array of pointers to
           struct sp_port, which will contain the ports found.*/
        struct sp_port **port_list;

        /* Call sp_list_ports() to get the ports. The port_list
           pointer will be updated to refer to the array created. */
        check(sp_list_ports(&port_list));

        /* Iterate through the ports. When port_list[i] is NULL
           this indicates the end of the list. */
        struct sp_port *resultPort;
        int i;
        for (i = 0; port_list[i] != NULL; i++) {
            struct sp_port *currentPort = port_list[i];
            
            /* Get the name of the port. */
            char *port_name = sp_get_port_name(currentPort);
            int usb_vid, usb_pid;
            sp_get_port_usb_vid_pid(currentPort, &usb_vid, &usb_pid);

            /* Check against the array of valid VID & PID's */            
            for (int j = 0; VENDOR_IDS[j] != NULL; j++) {
                if (usb_vid == VENDOR_IDS[j] && usb_pid == PRODUCT_IDS[j]) {
                    std::cout << "Found valid port:     " << port_name << std::endl;
                    strcpy(d_SerialMouse.portName, port_name);
                    std::cout << "Copied port name:     " << d_SerialMouse.portName << std::endl;
                    sp_copy_port(currentPort, &d_SerialMouse.port); // Copy the port to the struct port variable - Keeps read/write code simple
                }
            }
        }

        /* Free the array created by sp_list_ports(). */
        sp_free_port_list(port_list);
        return;
    }

    /**
     * @brief Start this function as a thread to continuously read data from the serial port without blocking the main thread.
     */
    void RunReadLoop()
    {
        if(!d_SerialMouse.isSerialMouseConnected){
            std::cerr << "[SM] isSerialMouseConnected returned false. Terminating Thread" << std::endl;
            return;
        }

        std::chrono::time_point<std::chrono::high_resolution_clock> lastLoopTime; // Log the time of the last loop to prevent uncontrolled loop speed
        while (d_SerialMouse.port)
        {
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastLoopTime);

            if (elapsedTime.count() < 500){
                // Sleep for the remaining time to hit 1ms loop time
                std::this_thread::sleep_for(std::chrono::microseconds(500 - elapsedTime.count()));
            }

            lastLoopTime = std::chrono::high_resolution_clock::now();

            ReadData();
        }

        std::cout << "[SM] 'port' returned false. Attempting to cleanly exit..." << std::endl;
        Disconnect();
        std::cout << "[SM] Thread terminated." << std::endl;
    }

public: 
    // Send a move command specifying the x and y coordinates
    void Move(int moveX, int moveY)
    {
        if (!d_SerialMouse.port)
        {
            throw std::runtime_error("Mouse Connection Error.");
        }

        std::string data = "M" + std::to_string(moveX) + ":" + std::to_string(moveY) + ":" + std::to_string(0) + "x";
        sp_blocking_write(d_SerialMouse.port, data.c_str(), data.length(), 0);
    }

    // Send a click event that has a humanized click duration
    void Fire()
    {
        if (!d_SerialMouse.port)
        {
            throw std::runtime_error("Mouse Connection Error.");
        }

        std::string data = "F" + std::to_string(55) + "x";
        sp_blocking_write(d_SerialMouse.port, data.c_str(), data.length(), 0);
    }
    
    /**
     * @brief Try to connect to a valid SerialMouse device
     */
    bool Connect()
    {
        FindDevice();
        
        // Check if the class was able to find a valid device
        if (d_SerialMouse.port == NULL){
            std::cout << "[SerialMouse] No valid device found!" << std::endl;
            return false;
        }

        check(sp_open(d_SerialMouse.port, SP_MODE_READ_WRITE ));

        check(sp_set_baudrate(d_SerialMouse.port, 115200));
        check(sp_set_bits(d_SerialMouse.port, 8));
        check(sp_set_parity(d_SerialMouse.port, SP_PARITY_NONE));
        check(sp_set_stopbits(d_SerialMouse.port, 1));
        check(sp_set_dtr(d_SerialMouse.port, SP_DTR_ON));
        check(sp_set_rts(d_SerialMouse.port, SP_RTS_ON));

        d_SerialMouse.isSerialMouseConnected = true;
        return true;
    }
    
    /**
     * @brief Disconnect from the serial port
     */
    void Disconnect(){
        if (d_SerialMouse.port)
        {
            check(sp_close(d_SerialMouse.port));
            sp_free_port(d_SerialMouse.port);
            d_SerialMouse.port = NULL;
            d_SerialMouse.isSerialMouseConnected = false;
        }
    }

    void StartThread()
    {
        if ( d_SerialMouse.isSerialMouseConnected == false ){
            std::cerr << "[SM] SerialMouse is not connected. Terminating Thread" << std::endl;
            return;
        }

        std::thread serialMouseReadThread(&SerialMouse::RunReadLoop, this);
        serialMouseReadThread.detach();
    }
};