#include <iostream>
#include <LimeSuite.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int main()
{

    // Declare a pointer to hold the LimeSDR device instance
    lms_device_t *device = nullptr;
    // Declare a variable to hold the return value of LimeSuite functions
    int ret;
    // Declare an array to hold the device information strings
    lms_info_str_t list[8];
    // Declare a pointer to hold the device information structure
    lms_dev_info_t *devInfo;

    // Get the list of available LimeSDR devices
    // The function returns the number of devices found
    // and fills the list array with their information
    // The list array can hold up to 8 devices
    int numDevices = LMS_GetDeviceList(list);

    if (numDevices <= 0)
    {
        std::cerr << "No LimeSDR devices found!" << std::endl;
        return -1;
    }

    std::cout << "Found " << numDevices << " device(s)." << std::endl;

    for (int i = 0; i < numDevices; ++i)
    {
        std::cout << "[" << i << "] " << list[i] << std::endl;
    }

    // Open the first device in the list
    ret = LMS_Open(&device, list[0], nullptr);
    if (ret != 0)
    {
        std::cerr << "Error opening device: " << LMS_GetLastErrorMessage() << std::endl;
        return -1;
    }
    std::cout << "Device opened successfully." << std::endl;

    // all trials here

    // initiate the device
    if (LMS_Init(device) != 0)
    {
        std::cerr << "Device initialization failed" << std::endl;
        LMS_Close(device);
        return -1;
    }
    std::cout << "Device initialized successfully." << std::endl;

    int numTXChannels = LMS_GetNumChannels(device, LMS_CH_TX);
    int numRXChannels = LMS_GetNumChannels(device, LMS_CH_RX);
    std::cout << "max TX Channels :" << numTXChannels << std::endl;
    std::cout << "max RX Channels :" << numTXChannels << std::endl;

    // lets try out RX functionalities
    // enable RX channel 0
    if (LMS_EnableChannel(device, LMS_CH_RX, 0, true) != 0)
    {
        std::cerr << "Failed to enable RX channel" << std::endl;
        LMS_Close(device);
        return -1;
    }
    std::cout << "RX channel 0 enabled successfully." << std::endl;


    // set RX sample rate to 1 MHz
    if (LMS_SetSampleRate(device, 30.72e6, 0) != 0)
    {
        std::cerr << "Failed to set RX sample rate" << std::endl;
        LMS_Close(device);
        return -1;
    }
    std::cout << "RX sample rate set to 30.72 MHz." << std::endl;

    // set center frequency to 2.4 GHz
    if (LMS_SetLOFrequency(device, LMS_CH_RX, 0, 2.4e9) != 0)
    {
        std::cerr << "Failed to set RX center frequency" << std::endl;
        LMS_Close(device);
        return -1;
    }
    std::cout << "RX center frequency set to 2.4 GHz." << std::endl;

    // set antenna
    // LMS_PATH_NONE = 0, ///<No active path (RX or TX)
    // LMS_PATH_LNAH = 1, ///<RX LNA_H port
    // LMS_PATH_LNAL = 2, ///<RX LNA_L port
    // LMS_PATH_LNAW = 3, ///<RX LNA_W port
    // LMS_PATH_TX1 = 1,  ///<TX port 1
    // LMS_PATH_TX2 = 2,   ///<TX port 2
    // LMS_PATH_AUTO = 255, ///<Automatically select port (if supported)
    if (LMS_SetAntenna(device, LMS_CH_RX, 0, LMS_PATH_LNAH) != 0)
    {
        if (LMS_SetAntenna(device, LMS_CH_RX, 0, LMS_PATH_LNAL) != 0)
        {
            if (LMS_SetAntenna(device, LMS_CH_RX, 0, LMS_PATH_LNAW) != 0)
            {
                std::cerr << "Failed to set RX antenna" << std::endl;
                LMS_Close(device);
                return -1;
            }
        }
    }
    std::cout << "RX antenna set to " << LMS_GetAntenna(device, LMS_CH_RX, 0) << std::endl;

    unsigned int gain = 20;
    // set gain in dB
    if (LMS_SetGaindB(device, LMS_CH_RX, 0, gain) != 0)
    {
        std::cerr << "Failed to set RX gain" << std::endl;
        LMS_Close(device);
        return -1;
    }
    unsigned int gainRead;
    LMS_GetGaindB(device, LMS_CH_RX, 0, &gainRead);
    std::cout << "RX gain was set to " << gain << " dB" << std::endl;

    float bandwidth = 15.36e6; // 15.36 MHz for 30.72 MSPS SamplingRate
    if (LMS_SetLPFBW(device, false, 0, bandwidth) != 0) 
    {
        std::cerr << "Failed to set RX LPF bandwidth" << std::endl;
        LMS_Close(device);
        return -1;
    }
    std::cout << "RX LPF bandwidth set to " << bandwidth / 1e6 << " MHz" << std::endl;

    // calibrate the RX channel
    if (LMS_Calibrate(device, LMS_CH_RX, 0, 15.36e6, 0) != 0)
    {
        std::cerr << "Failed to calibrate RX channel" << std::endl;
        LMS_Close(device);
        return -1;
    }
    std::cout << "RX channel calibrated successfully." << std::endl;


    // -----------------------------------------------------------------
    // Attempt signals

    // setup RX stream
    lms_stream_t rx_Stream;
    rx_Stream.channel = 0; // RX channel 0
    rx_Stream.isTx = false; // RX stream
    rx_Stream.fifoSize = 1024 * 1024; // Buffer size in samples
    rx_Stream.throughputVsLatency = 0.5; // Balance throughput and latency
    if (LMS_SetupStream(device, &rx_Stream) != 0)
    {
        std::cerr << "Failed to setup RX stream" << std::endl;
        LMS_Close(device);
        return -1;
    }
    std::cout << "RX stream setup successfully." << std::endl;

    // start RX stream
    if (LMS_StartStream(&rx_Stream) != 0)
    {
        std::cerr << "Failed to start RX stream" << std::endl;
        LMS_DestroyStream(device, &rx_Stream);
        LMS_Close(device);
        return -1;
    }
    std::cout << "RX stream started successfully." << std::endl;

    // receive stream
    float samples[1024];
    lms_stream_meta_t meta;
    // if(LMS_RecvStream(&rx_Stream, &samples, 1024, &meta, 1000) != 0)
    // {
    //     std::cerr << "Failed to receive samples" << std::endl;
    //     LMS_StopStream(&rx_Stream);
    //     LMS_DestroyStream(device, &rx_Stream);
    //     LMS_Close(device);
    //     return -1;
    // }
    // std::cout << "Received samples successfully." << std::endl;

    // Create named pipe (FIFO)
    const char* fifo_name = "/tmp/limesdr_fifo";
    if (mkfifo(fifo_name, 0666) == -1) {
        std::cerr << "Failed to create FIFO" << std::endl;
        LMS_StopStream(&rx_Stream);
        LMS_DestroyStream(device, &rx_Stream);
        LMS_Close(device);
        return -1;
    }
    std::cout << "FIFO created successfully." << std::endl;

    // Open FIFO for writing
    int fifo_fd = open(fifo_name, O_WRONLY);
    if (fifo_fd == -1) {
        std::cerr << "Failed to open FIFO for writing" << std::endl;
        LMS_StopStream(&rx_Stream);
        LMS_DestroyStream(device, &rx_Stream);
        LMS_Close(device);
        return -1;
    }
    std::cout << "FIFO opened for writing successfully." << std::endl;

    // continuous streaming
    while (true) {
        if(LMS_RecvStream(&rx_Stream, &samples, 1024, &meta, 1000) != 0)
        {
            std::cerr << "Failed to receive samples" << std::endl;
            LMS_StopStream(&rx_Stream);
            LMS_DestroyStream(device, &rx_Stream);
            LMS_Close(device);
            return -1;
        }
        std::cout << "Received samples successfully." << std::endl;

        // Write samples to FIFO
        ssize_t bytes_written = write(fifo_fd, samples, sizeof(samples));
        if (bytes_written == -1) {
            std::cerr << "Failed to write samples to FIFO" << std::endl;
            break; // Exit the loop on error
        }
        std::cout << "Wrote " << bytes_written << " bytes to FIFO." << std::endl;
        
        // Sleep for a while to simulate processing
        usleep(100000); // Sleep for 100 ms
    }

    // Cleanup
    close(fifo_fd);
    unlink(fifo_name);
    std::cout << "FIFO closed and unlinked successfully." << std::endl;

    // stop RX stream
    if (LMS_StopStream(&rx_Stream) != 0)
    {
        std::cerr << "Failed to stop RX stream" << std::endl;
        LMS_DestroyStream(device, &rx_Stream);
        LMS_Close(device);
        return -1;
    }
    std::cout << "RX stream stopped successfully." << std::endl;

    // Destroy the RX stream
    if (LMS_DestroyStream(device, &rx_Stream) != 0)
    {
        std::cerr << "Failed to destroy RX stream" << std::endl;
        LMS_Close(device);
        return -1;
    }
    std::cout << "RX stream destroyed successfully." << std::endl;
    // -----------------------------------------------------------------





    // // -------------------------------------------------------------------
    

    // double total_power = 0.0;
    // int samples_received_total = 0;
    // while (samples_received_total < total_samples)
    // {
    //     int samples_to_read = std::min(samples_per_buffer, total_samples - samples_received_total);
    //     int samples_received = LMS_RecvStream(&rx_stream, rx_buffer.data(), samples_to_read, nullptr, 1000);
    //     if (samples_received <= 0)
    //     {
    //         std::cerr << "Error receiving samples: " << samples_received << std::endl;
    //         break;
    //     }

    //     // Compute power for this buffer
    //     double buffer_power = 0.0;
    //     for (int i = 0; i < samples_received; ++i)
    //     {
    //         buffer_power += std::norm(rx_buffer[i]);
    //     }
    //     buffer_power /= samples_received;
    //     total_power += buffer_power * samples_received;

    //     // Write samples to file (I/Q as float pairs)
    //     outfile.write(reinterpret_cast<const char*>(rx_buffer.data()), samples_received * sizeof(std::complex<float>));

    //     samples_received_total += samples_received;
    //     std::cout << "Received " << samples_received_total << "/" << total_samples << " samples\r" << std::flush;
    // }
    // std::cout << std::endl;

    // // Compute and display average power
    // total_power /= samples_received_total;
    // std::cout << "Average signal power: " << 10 * std::log10(total_power) << " dBFS" << std::endl;

    // // Cleanup
    // outfile.close();
    // LMS_StopStream(&rx_stream);
    // LMS_DestroyStream(device, &rx_stream);

    // // --------------------------------------------------------------------


    // Close the device
    if (LMS_Close(device) != 0)
    {
        std::cerr << "Device failed to close" << std::endl;
        return 1;
    }
    std::cout << "Disconnected" << std::endl;

    return 0;
}