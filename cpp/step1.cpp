#include <iostream>
#include <LimeSuite.h>
#include <vector>
#include <cmath>
#include <complex>

int configure(lms_device_t* device) {
    // Set center frequency (e.g., 2.4 GHz)
    if (LMS_SetLOFrequency(device, LMS_CH_TX, 0, 2.4e9) != 0) {
        std::cerr << "LMS_SetLOFrequency failed (TX)" << std::endl;
        return 1;
    }
    std::cout << "center Frequency of Tx set to 2.4 GHz" << std::endl;
    if (LMS_SetLOFrequency(device, LMS_CH_RX, 0, 2.4e9) != 0) {
        std::cerr << "LMS_SetLOFrequency failed (RX)" << std::endl;
        return 1;
    }
    std::cout << "center Frequency of Rx set to 2.4 GHz" << std::endl;

    // Set sample rate (e.g., 2 MHz)
    if (LMS_SetSampleRate(device, 2e6, 0) != 0) {
        std::cerr << "LMS_SetSampleRate failed" << std::endl;
        LMS_Close(device);
        return 1;
    }
    std::cout << "Sample Rate set to 2.0 MHz" << std::endl;

    // Enable channels
    if (LMS_EnableChannel(device, LMS_CH_TX, 0, true) != 0) {
        std::cerr << "LMS_EnableChannel failed (TX)" << std::endl;
        LMS_Close(device);
        return 1;
    }
    std::cout << "Tx Channel 0 enabled" << std::endl;
    if (LMS_EnableChannel(device, LMS_CH_RX, 0, true) != 0) {
        std::cerr << "LMS_EnableChannel failed (RX)" << std::endl;
        LMS_Close(device);
        return 1;
    }
    std::cout << "Rx Channel 0 enabled" << std::endl;

    return 0;
}

int main() {
    lms_device_t* device = nullptr;
    int ret;
    lms_info_str_t list[8];
    lms_dev_info_t* devInfo;

    int numDevices = LMS_GetDeviceList(list);

    if (numDevices <= 0) {
        std::cerr << "No LimeSDR devices found!" << std::endl;
        return -1;
    }

    std::cout << "Found " << numDevices << " device(s)." << std::endl;

    for (int i = 0; i < numDevices; ++i) {
        std::cout << "[" << i << "] " << list[i] << std::endl;
    }

    if (numDevices > 0) {
        ret = LMS_Open(&device, list[0], nullptr);
        if (ret != 0) {
            std::cerr << "Error opening device: " << LMS_GetLastErrorMessage() << std::endl;
            return -1;
        }
        std::cout << "Device opened successfully." << std::endl;

        // ... (SDR configuration and data transfer code will go here)
        if(configure(device)!=0) {
            std::cout << "Device configuration failed" <<std::endl;
            LMS_Close(device);
            return -1;
        }

        
        if (LMS_Close(device)!=0) {
            std::cerr << "Device failed to close" << std::endl;
            return 1;
        } 
        std::cout << "Disconnected" << std::endl;
    } else {
    std::cout << "No devices found" << std::endl;
    }

    return 0;
}