/**
 * @file
 * @author Paul Zander <Paul.Zander at uni-rostock.de>
 * @version 0.2
 */

#ifndef JRLNAOQIWRAPPER_SRC_CAMERA_NAOCAMERA_H_
#define JRLNAOQIWRAPPER_SRC_CAMERA_NAOCAMERA_H_

#include <iostream>
#include <vector>
#include <cstring> //memset
#include <cassert> //assert
#include <unistd.h>
#include <cerrno>
#include <malloc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <cstdlib>
#include <linux/videodev2.h>



namespace NaoCamera {

class Settings {
public:

    Settings(const int &exp, const int &bright, const int &gain, const int &cont,
             const int &sat, const int &sharp, const int &white) : exposure(exp),
        brightness(bright), gain(gain), contrast(cont), saturation(sat),
        sharpness(sharp), whiteBalance(white) {};

    /// @param value default value
    explicit Settings(const int &value) : exposure(value), brightness(value),
        gain(value), contrast(value), saturation(value), sharpness(value),
        whiteBalance(value) {};

    /// getControl(V4L2_CID_EXPOSURE);
    int exposure;

    /// getControl(V4L2_CID_BRIGHTNESS);
    int brightness;

    /// getControl(V4L2_CID_GAIN);
    int gain;

    /// getControl(V4L2_CID_CONTRAST)
    int contrast;

    /// getControl(V4L2_CID_SATURATION)
    int saturation;

    /// getControl(V4L2_CID_SHARPNESS)
    int sharpness;

    /// getControl(V4L2_CID_DO_WHITE_BALANCE)
    int whiteBalance;

    /// full output
    friend std::ostream &operator<<(std::ostream &out, const Settings &settings);
};

struct buffer {
    /// start of the buffer
    void *start;
    /// maximum length
    std::size_t length;
};

enum class IO_METHOD {
    READ,
    MMAP,
    USERPTR
};

class Camera {
private:
    int               fd;
    buffer           *buffers;
    unsigned int      n_buffers;
    std::string dev_name;
    unsigned int      width;
    unsigned int      height;
    bool              isNao;
    bool              running;
    v4l2_buf_type     type;
    IO_METHOD         io;
    const v4l2_memory memory;
    const unsigned int pixelformat;

    //     v4l2_queryctrl queryctrl;
    //     v4l2_querymenu querymenu;

    /**
     * wrapper for extended error messages
     * @param error the error string
     */
    static const std::string debug_error(const std::string &error) {
        return error + ": " + std::to_string(static_cast<long long int>(errno)) + ", " + strerror(errno);
    }


    /**
     * wrapper for extended error messages
     * @param error the error string
     */
    const std::string debug_devname(const std::string &error) const {
        return dev_name + " " + error;
    }

    friend class RawImage;

    void init_mmap(void);
    void init_userp(unsigned int buffer_size);
    void init_read(unsigned int buffer_size);

    virtual std::size_t length(v4l2_buffer const *const buf) const;

public:
    virtual void *start(v4l2_buffer const *const  buf) const;
    Camera(const Settings &defaultsettings, const int &device);
    virtual ~Camera();

    int  getControl(const int &id) const;
    bool setControl(const int &id, int &value);
    bool setControl(const int &id, const int &value);
    void set(int prop, std::string propname, int val);

    bool get_image(v4l2_buffer *&buf);
    bool release_image(v4l2_buffer const *buf) const;

    void set_parameter();
    bool start_capturing() const;
    bool stop_capturing() const;

    static bool ToggleCamera();

    void list_inputs(std::ostream &out) const;
    void list_controls(std::ostream &out) const;
    void print_control(std::ostream &out, const v4l2_queryctrl &queryctrl) const;
    void enumerate_menu(std::ostream &out, const v4l2_queryctrl &queryctrl) const;

    std::vector<Settings>          settings;

    // id of the current active cam \n 0 = upper, 1 = lower
    int  activeCam;
};

/**
 * wrapper for ioctl to handle EINTR
 * @param fd an open file descriptor
 * @param request a request code number
 * @param arg either an integer value, possibly unsigned (going to the driver) or a pointer to data (either going to the driver, coming back from the driver, or both).
 */
inline int xioctl(int fd, unsigned int request, const void *arg) {
    int r;

    do {
        r = ioctl(fd, request, arg);
    } while (-1 == r && EINTR == errno);
    if (errno == EBUSY) {
        std::cerr << "device is busy" << std::endl;
    }
    if (errno == ERANGE) {
        std::cerr << "value is out of range" << std::endl;
    }
    return r;
}

}


//this is bad
//do debug_error and debug_devname in catch block
class camexc : public std::exception {
    char const *msg;
public:
    explicit camexc(const char *foo): msg(foo) {};
    explicit camexc(const std::string &foo): msg(foo.c_str()) {};
    char const *what() const throw() {
        return msg;
    }

};

#endif  // JRLNAOQIWRAPPER_SRC_CAMERA_NAOCAMERA_H_

// vim: set ts=4 sw=4 sts=4 expandtab:
