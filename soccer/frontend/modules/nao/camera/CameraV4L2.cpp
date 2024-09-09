//! @file
#include "CameraV4L2.hpp"

extern "C" {
#include <fcntl.h>            // ::open
#include <linux/usb/video.h>  // UVC Controls
#include <linux/uvcvideo.h>   // UVC ExtUnit Controls
#include <linux/videodev2.h>  // V4L2 Controls
#include <sys/ioctl.h>        // ::ioctl
#include <sys/mman.h>         // ::mmap
#include <unistd.h>           // ::close
}
#include <cassert>
#include <cerrno>
#include <chrono>
#include <cstring>  // std::memset
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <thread>

//#include <easy/profiler.h>

using namespace std::chrono;

#define ioctlf(fd, mode, buf)                                                                 \
    if (-1 == ::ioctl(fd, mode, buf)) {                                                       \
        std::cerr << "(ERROR) " << device << ": " #mode " (" << std::to_string(_IOC_NR(mode)) \
                  << "): " << std::strerror(errno) << std::endl;                              \
        throw std::runtime_error(device + ": " #mode ": " + std::strerror(errno));            \
    }

CameraV4L2::~CameraV4L2() {
    closeDevice();
}

void CameraV4L2::openDevice() {
    if (fd != -1)
        return;  // already open

    // fd = ::open(_device.c_str(), O_RDWR | O_CLOEXEC);
    fd = ::open(device.c_str(), O_RDWR | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) {
        fd = -1;
        throw std::runtime_error(device + ": open fails: " + std::strerror(errno));
    }
}

void CameraV4L2::closeDevice() {
    if (fd == -1)
        return;  // already close

    // Force stop is running
    try {
        stop();
    } catch(...) {

    }

    fd = ::close(fd);
    if (fd < 0) {
        fd = -1;
        throw std::runtime_error(device + ": close fails: " + std::strerror(errno));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    fd = -1;
    //_format = {{0,0},0};
}

bool CameraV4L2::isOpen() const {
    return (fd != -1);
}

void CameraV4L2::setParameter(std::uint32_t param, std::int32_t value) {
    struct v4l2_control control;
    std::memset(&control, 0, sizeof(control));
    control.id = param;
    control.value = value;
    ioctlf(fd, VIDIOC_S_CTRL, &control);
}

std::int32_t CameraV4L2::getParameter(std::uint32_t param) {
    struct v4l2_control control;
    std::memset(&control, 0, sizeof(control));
    control.id = param;
    ioctlf(fd, VIDIOC_G_CTRL, &control);
    return control.value;
}

void CameraV4L2::setExtUnit(std::uint8_t reg, std::int16_t value) {
    std::uint8_t data[2];
    data[0] = value & 0xff;
    data[1] = (value >> 8) & 0xff;

    struct uvc_xu_control_query queryctrl;
    std::memset(&queryctrl, 0, sizeof(queryctrl));
    queryctrl.unit = 3;
    queryctrl.selector = reg;
    queryctrl.query = UVC_SET_CUR;
    queryctrl.size = 2;
    queryctrl.data = data;
    ioctlf(fd, UVCIOC_CTRL_QUERY, &queryctrl);
}

std::uint16_t CameraV4L2::readDeviceRegister(std::uint16_t addr) const {
    struct uvc_xu_control_query xu_query;
    std::memset(&xu_query, 0, sizeof(xu_query));
    xu_query.unit = 3;
    xu_query.selector = DEVICE_REGISTER;
    xu_query.query = UVC_SET_CUR;
    xu_query.size = 5;

    std::uint8_t data[5];
    std::memset(data, 0, 5);
    data[0] = 0;  // Read
    data[1] = addr >> 8;
    data[2] = addr & 0xff;
    xu_query.data = data;
    ioctlf(fd, UVCIOC_CTRL_QUERY, &xu_query);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    xu_query.query = UVC_GET_CUR;
    ioctlf(fd, UVCIOC_CTRL_QUERY, &xu_query);

    return (std::uint16_t(data[3]) << 8) | std::uint16_t(data[4]);
}

void CameraV4L2::writeDeviceRegister(std::uint16_t addr, std::uint16_t value) {
    struct uvc_xu_control_query xu_query;
    std::memset(&xu_query, 0, sizeof(xu_query));
    xu_query.unit = 3;
    xu_query.selector = DEVICE_REGISTER;
    xu_query.query = UVC_SET_CUR;
    xu_query.size = 5;

    std::uint8_t data[5];
    data[0] = 1;  // Write
    data[1] = addr >> 8;
    data[2] = addr & 0xff;
    data[3] = value >> 8;
    data[4] = value & 0xff;
    xu_query.data = data;
    ioctlf(fd, UVCIOC_CTRL_QUERY, &xu_query);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void CameraV4L2::init() {
    openDevice();
    updateDeviceResolution();
    updateDeviceFrameRate();

    setParameter(V4L2_CID_BRIGHTNESS, 0);
    setParameter(V4L2_CID_CONTRAST, 32);
    setParameter(V4L2_CID_SATURATION, 64);
    setParameter(V4L2_CID_HUE, 0);
    setParameter(V4L2_CID_SHARPNESS, 4);

    setExtUnit(ExtensionUnit::HORIZONTAL_FLIP, 0);  // disable Horizontal Flip
    setExtUnit(ExtensionUnit::VERTICAL_FLIP, 0);    // disable Vertical Flip
    setExtUnit(ExtensionUnit::TEST_PATTERN, 0x0);   // disable Test Pattern

    setParameter(V4L2_CID_EXPOSURE_AUTO, V4L2_EXPOSURE_AUTO);
    setParameter(V4L2_CID_AUTO_WHITE_BALANCE, 1);
    setParameter(V4L2_CID_FOCUS_AUTO, 1);  // enable auto focus
}

void CameraV4L2::start() {
    if (running)
        return;
    initMmapStreaming();
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctlf(fd, VIDIOC_STREAMON, &type);
    running = true;
}

void CameraV4L2::stop() {
    if (!running)
        return;
    running = false;
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctlf(fd, VIDIOC_STREAMOFF, &type);
    unInitMmapStreaming();
}

bool CameraV4L2::isRunning() const {
    return running;
}

int64_t CameraV4L2::getImage(uint8_t* buffer) {
    //EASY_FUNCTION();

    v4l2_buffer v4l2_buffer = lockKernelBuffer();
    std::memcpy(buffer, v4l2Buffers[v4l2_buffer.index].first, bufferSize());
    int64_t timestamp = v4l2_buffer.timestamp.tv_sec * 1'000'000ll + v4l2_buffer.timestamp.tv_usec;
#if 0
	// we don't care about realtime timestamps
    if (timestamp < 1'555'000'000'000'000ll) {
        std::cerr << "Cam not using real time! (Or bios battery empty)" << std::endl;
    }
#endif
    ioctlf(fd, VIDIOC_QBUF, &v4l2_buffer);
    return timestamp;
}

void CameraV4L2::updateDeviceResolution() {
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  // for a video device
    fmt.fmt.pix.field = V4L2_FIELD_NONE;     // not interlaced camera
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.colorspace = V4L2_COLORSPACE_JPEG;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    ioctlf(fd, VIDIOC_S_FMT, &fmt);
}

void CameraV4L2::updateDeviceFrameRate() {
    struct v4l2_streamparm parm;
    std::memset(&parm, 0, sizeof(parm));
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parm.parm.capture.timeperframe.numerator = 1;
    parm.parm.capture.timeperframe.denominator = fps;
    // indicates that the device can vary its frame rate
    parm.parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
    parm.parm.capture.extendedmode = 0;
    ioctlf(fd, VIDIOC_S_PARM, &parm);
}

void CameraV4L2::initMmapStreaming() {
    // First request a number of buffer.
    struct v4l2_requestbuffers req;
    std::memset(&req, 0, sizeof(req));
    req.count = ringBufferSize;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    ioctlf(fd, VIDIOC_REQBUFS, &req);
    if (req.count != ringBufferSize) {
        throw std::runtime_error(device +
                                 ": Insufficient memory to create "
                                 "enough RAW buffer in the kernel "
                                 "ring buffer.");
    }

    // Get pointer and size of each one
    for (std::uint32_t i = 0; i < ringBufferSize; ++i) {
        struct v4l2_buffer buf;
        std::memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        ioctlf(fd, VIDIOC_QUERYBUF, &buf);

        // MMAP them
        void* startAddr = ::mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if (MAP_FAILED == startAddr) {
            throw std::runtime_error(device + ": Cannot mmap kernel buffer " + std::to_string(i) + ": " +
                                     strerror(errno));
        }
        v4l2Buffers.emplace_back(std::make_pair(startAddr, buf.length));

        // Enqueue them
        ioctlf(fd, VIDIOC_QBUF, &buf);
    }
}

void CameraV4L2::unInitMmapStreaming() {
    bool success = true;
    for (auto& i : v4l2Buffers) {
        if (::munmap(i.first, i.second)) {
            success = false;
        }
    }
    v4l2Buffers.clear();
    if (!success) {
        throw std::runtime_error(device + ": Error when unmapping RAW buffer: " + strerror(errno));
    }

    // Remove all buffer if any i.e. request 0 buffer
    struct v4l2_requestbuffers req;
    std::memset(&req, 0, sizeof(req));
    req.count = 0;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    ioctlf(fd, VIDIOC_REQBUFS, &req);
    if (req.count != 0) {
        throw std::runtime_error(device + ": Can't remove all previously allocated buffer.");
    }
}

v4l2_buffer CameraV4L2::lockKernelBuffer() {
    v4l2_buffer buffer;
    std::memset(&buffer, 0, sizeof(struct v4l2_buffer));
    buffer.index = 0;
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    //! 1. First try to DQBUF a buffer if any.
    int result = ::ioctl(fd, VIDIOC_DQBUF, &buffer);
    if (-1 == result && EAGAIN == errno) {
        //! 2. Otherwise wait for a new frame.
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        struct timeval timeout {
            .tv_sec = 1, .tv_usec = 0
        };
        int r = ::select(fd + 1, &fds, nullptr, nullptr, &timeout);
        if (0 == r) {
            std::cerr << "(ERROR) " << device << ": Timeout (1s) !" << std::endl;
            throw std::runtime_error(device + ": Can't get new image from driver: timeout (1s) on select");
        } else if (-1 == r) {
            std::cerr << "(ERROR) " << device << ": Can't get new image from driver: " << std::strerror(errno)
                      << std::endl;
            throw std::runtime_error(device + ": Can't get new image from driver: " + std::strerror(errno));
        }
        result = ::ioctl(fd, VIDIOC_DQBUF, &buffer);
    }
    if (0 != result) {
        std::cerr << "(ERROR) " << device << ": VIDIOC_DQBUF (" << std::to_string(_IOC_NR(VIDIOC_DQBUF))
                  << "): " << std::strerror(errno) << std::endl;
        throw std::runtime_error(device + ": Cannot dequeue v4l2 buffer from kernel ring buffer: " + strerror(errno));
    }
    return buffer;
}
