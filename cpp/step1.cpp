#include <iostream>
#include <LimeSuite.h>
#include <vector>
#include <cmath>
#include <complex>


// 100 kHz sine wave // baseband frequency // message content frequency
const double baseband_frequency = 100e3; 
// 2.4 GHz tx/rx frequency
const double carrier_frequency = 2.4e9;
// messaging / reading frequency
const double sampling_rate = 2e6;
// choice of channel
const short channel = 0;


int configure(lms_device_t* device) {
    // Set center frequency (e.g., 2.4 GHz)
    if (LMS_SetLOFrequency(device, LMS_CH_TX, 0, carrier_frequency) != 0) {
        std::cerr << "LMS_SetLOFrequency failed (TX)" << std::endl;
        return 1;
    }
    std::cout << "center Frequency of Tx set to 2.4 GHz" << std::endl;
    if (LMS_SetLOFrequency(device, LMS_CH_RX, 0, carrier_frequency) != 0) {
        std::cerr << "LMS_SetLOFrequency failed (RX)" << std::endl;
        return 1;
    }
    std::cout << "center Frequency of Rx set to 2.4 GHz" << std::endl;

    // Set sample rate (e.g., 2 MHz)
    if (LMS_SetSampleRate(device, sampling_rate, 0) != 0) {
        std::cerr << "LMS_SetSampleRate failed" << std::endl;
        LMS_Close(device);
        return 1;
    }
    std::cout << "Sample Rate set to 2.0 MHz" << std::endl;

    // Enable channels
    if (LMS_EnableChannel(device, LMS_CH_TX, channel, true) != 0) {
        std::cerr << "LMS_EnableChannel failed (TX)" << std::endl;
        LMS_Close(device);
        return 1;
    }
    std::cout << "Tx Channel 0 enabled" << std::endl;
    if (LMS_EnableChannel(device, LMS_CH_RX, channel, true) != 0) {
        std::cerr << "LMS_EnableChannel failed (RX)" << std::endl;
        LMS_Close(device);
        return 1;
    }
    std::cout << "Rx Channel 0 enabled" << std::endl;

    return 0;
}

int transmit(lms_device_t* device) {

    // generate sine wave
    const int no_of_samples = 1024;
    // samples to transmit
    std::vector<std::complex<float>> tx_samples(no_of_samples);
    for (int i=0; i < no_of_samples; ++i) {
        double time = i / sampling_rate;
        tx_samples[i] = std::complex<float>(std::sin(2 * M_PI * baseband_frequency * time), 0.0f);
    }

    // prepare transmit stream params
    lms_stream_t stream_tx;
    stream_tx.channel = channel;
    stream_tx.isTx = true;
    stream_tx.dataFmt = lms_stream_t::LMS_FMT_F32;    
    lms_stream_meta_t meta_tx;

    // setup sending stream
    if (LMS_SetupStream(device, &stream_tx) != 0) {
        std::cerr << "LMS_SetupStream failed (Tx)" << std::endl;
        LMS_Close(device);
        return 1;
    }
    std::cout << "LMS_SetupStream successful (Tx)" << std::endl;

    //transmit the data
    int tx_sent = LMS_SendStream(&stream_tx, tx_samples.data(), no_of_samples, &meta_tx, 1000);
    std::cerr << "LMS_SendStream sent " << tx_sent << " of " << no_of_samples << std::endl;

    // stop tranmission
    LMS_StopStream(&stream_tx);
    LMS_DestroyStream(device, &stream_tx);

    return 0;
}

int receive(lms_device_t* device) {

    // prepare receive stream params
    const int no_of_samples = 1024;
    std::vector<std::complex<float>> rx_samples(no_of_samples);
    lms_stream_t stream_rx;
    stream_rx.channel = channel;
    stream_rx.isTx = false;
    stream_rx.dataFmt = lms_stream_t::LMS_FMT_F32;    
    lms_stream_meta_t meta_rx;

    // setup receiving stream
    if(LMS_SetupStream(device, &stream_rx) != 0) {
        std::cerr << "LMS_SetupStream failed (Rx)" << std::endl;
        LMS_Close(device);
        return 1;
    }

    // receive some data
    int rx_received = LMS_RecvStream(&stream_rx, rx_samples.data(), no_of_samples, &meta_rx, 1000);
    if (rx_received > 0) {
        std::cout << "Received " << rx_received << " samples" << std::endl;
        // Process received samples (e.g., calculate RSSI)
        std::cout << rx_samples.data() << std::endl;
    } else {
        std::cerr << "LMS_RecvStream failed (Rx)" << std::endl;
        std::cerr << LMS_GetLastErrorMessage() << std::endl;
    }
    
    // stop reception
    LMS_StopStream(&stream_rx);
    LMS_DestroyStream(device, &stream_rx);

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

        // tx
        if(transmit(device)!=0) {
            std::cout << "Transmit failed" <<std::endl;
            LMS_Close(device);
            return -1;
        }

        // rx
        if(receive(device)!=0) {
            std::cout << "Receipt failed" <<std::endl;
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