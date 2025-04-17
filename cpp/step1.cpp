#include <iostream>
#include <LimeSuite.h>

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

    ret = LMS_Open(&device, list[0], nullptr);
    if (ret != 0) {
        std::cerr << "Error opening device: " << LMS_GetLastErrorMessage() << std::endl;
        return -1;
    }
    std::cout << "Device opened successfully." << std::endl;

    devInfo = LMS_GetDeviceInfo(&device)

    return 0;
}