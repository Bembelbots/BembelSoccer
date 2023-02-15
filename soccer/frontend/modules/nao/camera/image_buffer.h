#pragma once

#include <cstdint>
#include <atomic>
#include <bitset>
#include <string_view>
#include <vector>

#include <representations/camera/camera.h>
#include <framework/image/camimage.h>

/**
 * ringbuffer for CamImages
 */
class ImageBuffer {
public:
    /**
     * constructor
     * @param size number of images in the buffer
     * @param cam  ID of camera to which this buffer belongs (TOP_CAMERA/BOTTOM_CAMERA)
     */
    ImageBuffer() = default;
    ~ImageBuffer();

    void initialize(const size_t &size, const int &cam, bool sim = false);
    void deinit();

    /// returns most recent image
    CamImage &getImage();

    /**
     * @return reference to a locked CamImage from the ringbuffer,
     * that can be populated with a new camera image
     * 
     * NEVER modify or free the images data pointer!
     */
    CamImage &getCaptureBuffer();

    /**
     * unlocks CamImage previously obtained by getCaptureBuffer() and marks it as
     * next image to be returned by getImage()
     */
    void releaseCaptureBuffer();

private:
    /// image size in bytes
    static constexpr size_t imgSize{camera::w * camera::h * camera::bpp};
    std::string_view camName;
    bool simulator{false};
    std::vector<CamImage> data;

    // store most recent new and freed image pointer for increased performance
    std::atomic<uint32_t> curImg{0}, lastImg{1}, capture{0};
};

// vim: set ts=4 sw=4 sts=4 expandtab:
