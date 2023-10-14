#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct v4l2_buffer;

//! @brief Interface for a V4L2 Camera object.
class CameraV4L2 {
public:
    enum ExtensionUnit : uint8_t {
        VERSION           = 0x03,
        AVERAGE_LUMINANCE = 0x08,
        TEST_PATTERN      = 0x0a,
        HORIZONTAL_FLIP   = 0x0c,
        VERTICAL_FLIP     = 0x0d,
        DEVICE_REGISTER   = 0x0e
    };

    //! @brief Default constructor.
    //! @param[in] device Device file to use.
    //! @param[in] format Default format to use at init.
    //! @param[in] verbose Enable verbose log.
    CameraV4L2(std::string device, uint32_t width, uint32_t height, uint32_t fps)
        : device(std::move(device)), width(width), height(height), fps(fps) {}
    ~CameraV4L2();
    CameraV4L2(const CameraV4L2&) = delete;
    CameraV4L2(CameraV4L2&&) = delete;
    CameraV4L2& operator=(const CameraV4L2&) = delete;
    CameraV4L2& operator=(CameraV4L2&&) = delete;

    //! @brief Opens the device file.
    //! @throw std::runtime_error file can't be open.
    void openDevice();

    //! @brief Closes the device file.
    void closeDevice();

    //! @brief Checks if the device file is open.
    //! @return true if device file is open, false otherwise.
    bool isOpen() const;

    //! @brief Gets the expected size of an Image.
    //! @return the size of an image in byte.
    inline std::uint32_t bufferSize() const noexcept {
        return width * height * 2u;
    }

    //! @brief Get the current value of a Parameter.
    //! @param[in] param Parameter requested.
    std::int32_t getParameter(std::uint32_t param);

    //! @brief Updates the current value of a Parameter.
    //! @param[in] param Parameter requested.
    //! @param[in] value New value requested.
    void setParameter(std::uint32_t param, std::int32_t value);

    //! @brief Updates the current value of a Register.
    //! @param[in] reg Register requested.
    //! @param[in] value New value requested.
    void setExtUnit(std::uint8_t reg, std::int16_t value);

    //! @brief Read the current value of a device register.
    //! @param[in] addr Register requested.
    std::uint16_t readDeviceRegister(std::uint16_t addr) const;

    //! @brief Write a new value of a device register.
    //! @param[in] addr Register requested.
    //! @param[in] value new value of a register.
    void writeDeviceRegister(std::uint16_t addr, uint16_t value);

    //! @brief Initializes the Camera.
    void init();

    //! @brief Starts the Camera stream.
    void start();

    //! @brief Stops the Camera stream.
    void stop();

    //! @brief Checks if the Camera is streaming.
    //! @return true if camera is streaming, false otherwise.
    bool isRunning() const;

    //! @brief Gets Image.
    //! @return Timestamp of the image.
    int64_t getImage(uint8_t *buffer);

    //! @brief Name of the device file.
    const std::string device;

protected:
    //! @brief Updates Device Resolution.
    //! @param[in] bypassCheck If true don't check previous value and update anyway.
    void updateDeviceResolution();

    //! @brief Updates Device Frame Rate.
    //! @param[in] bypassCheck If true don't check previous value and update anyway.
    void updateDeviceFrameRate();

    //! @brief Initializes V4L2 ring buffer and Mmap them.
    void initMmapStreaming();
    //! @brief Unmmaps all buffer and remove them.
    void unInitMmapStreaming();

    //! @brief Gets a Image from the V4L2 ring buffer.
    //! @return the mmapped V4L2 kernel buffer.
    v4l2_buffer lockKernelBuffer();

    //! @brief Stores the file descriptor.
    std::int32_t fd = -1;

    //! @brief Stores the list of mmap V4L2 kernel buffer.
    std::vector<std::pair<void *, std::uint32_t>> v4l2Buffers;

    //! @brief Stores if stream is running.
    bool running = false;

    //! @brief Stores the format requested.
    const uint32_t width;
    const uint32_t height;
    const uint32_t fps;

    //! @brief Stores the buffer number requested.
    std::uint32_t ringBufferSize = 4;
};
