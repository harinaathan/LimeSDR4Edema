#include <iostream>
#include <LimeSuite.h>

int main() {
    lms_device_t* device = nullptr;
    int ret;

    // Find available devices
    lms_info_str_t* list[8];
    int numDevices = LMS_GetDeviceList(list);

    if (numDevices <= 0) {
        std::cerr << "No LimeSDR devices found!" << std::endl;
        return -1;
    }

    std::cout << "Found " << numDevices << " device(s)." << std::endl;
    for (int i = 0; i < numDevices; ++i) {
        std::cout << "[" << i << "] " << list[i] << std::endl;
    }

    // Open the first device in the list
    ret = LMS_Open(&device, list[0], nullptr);
    LMS_FreeDeviceList(list); // Free the device list

    if (ret != 0) {
        std::cerr << "Error opening device: " << LMS_GetError(device) << std::endl;
        return -1;
    }

    std::cout << "Device opened successfully." << std::endl;

    // Get device name
    char name[64];
    LMS_GetDeviceName(device, name);
    std::cout << "Device Name: " << name << std::endl;

    // Get firmware version
    lms_firmware_info_t fw_info;
    LMS_GetFirmwareInfo(device, &fw_info);
    std::cout << "Firmware Version: " << fw_info.firmwareVersion << ", "
              << "Build Date: " << fw_info.buildDate << ", "
              << "Library Version: " << fw_info.libraryVersion << std::endl;

    // Close the device
    LMS_Close(device);
    std::cout << "Device closed." << std::endl;

    return 0;
}