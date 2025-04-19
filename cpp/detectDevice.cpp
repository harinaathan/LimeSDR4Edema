#include <iostream>
#include <LimeSuite.h>

using namespace std;
using namespace lime;

int main() {
    vector<Device::Info> devices;
    int ret = Device::enumerate(devices);

    if (ret != 0) {
        cerr << "Error enumerating devices: " << ret << endl;
        return 1;
    }

    if (devices.empty()) {
        cout << "No LimeSDR devices found." << endl;
    } else {
        cout << "Found the following LimeSDR devices:" << endl;
        for (const auto& devInfo : devices) {
            cout << "  " << devInfo.deviceName << " [Serial: " << devInfo.serial << "]" << endl;
        }
    }

    return 0;
}