/**
 * @file
 * @author Paul Zander <Paul.Zander at uni-rostock.de>
 * @version 0.2
 */

#include "NaoCamera.h"

#include <framework/logger/logger.h>

#include <exception>
#include <iterator>
#include <iomanip>

#include <framework/util/assert.h>
#include <representations/bembelbots/constants.h>
#include <representations/camera/camera.h>


//debug output
const bool  DEBUGGING        = true;
const bool  CHECKS           = false;
const bool  VERBOSE_CAMERA   = false;
const bool  show_inputs      = false;
const bool  show_controls    = false;
const bool  nonblocking      = true;

//how many buffers we should try to queue
const unsigned buffer_count  = 3;


std::ostream &Log = std::clog;

template <typename T>
inline void LOG(const T &txt) {
    Log << txt << '\n';
}

inline void CLEAR(void *x) {
    memset(&(x), 0, sizeof(x));
}


/**
 * dequeue a buffer
 * @param buf the buf
 */
bool NaoCamera::Camera::get_image(v4l2_buffer *&buf) {
    switch (io) {
    case IO_METHOD::READ:
        if (-1 == read(fd, buffers[0].start, buffers[0].length)) {
            switch (errno) {
            case EAGAIN:
                return 0;

            case EIO:
            /* Could ignore EIO, see spec. */

            /* fall through */

            default:
                std::cerr << debug_error("read") << std::endl;
                return false;
            }
        }

        break;

    // currently we do this!
    case IO_METHOD::MMAP:
        unsigned int i;
        for (i = 0; i < buffer_count - 1; ++i) {
            if (nonblocking) {
                fd_set fds = {{0}};
                timeval tv = {1, 0};
                FD_SET(fd, &fds);
                int r = select(fd + 1, &fds, NULL, NULL, &tv);
                if (-1 == r) {
                    if (errno == EINTR || errno == EAGAIN) {
                        continue;
                    }
                    std::cerr << debug_error("select") << std::endl;
                    return false;
                }
                if (0 == r) {
                    LOG("select timeout");
                    continue;
                }
            }

            memset(buf, 0, sizeof(v4l2_buffer));
            buf->type = type;
            buf->memory = memory;

            if (-1 == xioctl(fd, VIDIOC_DQBUF, buf)) {
                switch (errno) {
                case EAGAIN:
                    continue;
                case EIO:
                default:
                    std::cerr << debug_error("VIDIOC_DQBUF") << std::endl;
                    return false;
                }
            } else {
                return true;
            }
        }
        return false;

        break;

    case IO_METHOD::USERPTR:
        CLEAR(buf);

        buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf->memory = V4L2_MEMORY_USERPTR;

        if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
            switch (errno) {
            case EAGAIN:
                return 0;

            case EIO:
            /* Could ignore EIO, see spec. */

            /* fall through */

            default:
                std::cerr << debug_error("VIDIOC_DQBUF") << std::endl;
                return false;
            }
        }

        for (i = 0; i < n_buffers; ++i)
            if (buf->m.userptr == (unsigned long) buffers[i].start
                    && buf->length == buffers[i].length) {
                break;
            }

        assert(i < n_buffers);

        break;
    }
    return true;
}

/**
 * queue a buffer
 * @param buf the buf
 */
bool NaoCamera::Camera::release_image(v4l2_buffer const *buf) const {
    if (-1 == xioctl(fd, VIDIOC_QBUF, buf)) {
        std::cerr << debug_error("VIDIOC_QBUF") << std::endl;
        return false;
    }
    return true;
}

NaoCamera::Camera::Camera(const Settings &defaultsettings,
                          const int &device) : fd(0), buffers(NULL), n_buffers(buffer_count),
    dev_name("/dev/video0"), width(camera::w), height(camera::h),
    type(V4L2_BUF_TYPE_VIDEO_CAPTURE), io(IO_METHOD::MMAP),
    memory(V4L2_MEMORY_MMAP), pixelformat(V4L2_PIX_FMT_YUYV), activeCam(device) {

    if (VERBOSE_CAMERA) {
        LOG_INFO << "defaultsettings:\n" <<  defaultsettings;
    }

    if (device == 0) {
        LOG_INFO << "Init TOP camera";
        dev_name = "/dev/video0";
    } else if (device == 1) {
        LOG_INFO << "Init BOTTOM camera";
        dev_name = "/dev/video1";
    } else {
        throw (camexc(debug_error("camera id unknown!")));
    }

    if (pixelformat == V4L2_PIX_FMT_RGB565) {
        type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
    }

    settings.resize(1, defaultsettings); //we have 2 cameras
    try {
        if (CHECKS) {
            struct stat st;
            if (-1 == stat(dev_name.c_str(), &st)) {
                throw (camexc("Cannot identify '" + dev_name +
                              "'\ncritical error: no camera found"));
            }
            if (!S_ISCHR(st.st_mode)) {
                throw (camexc(debug_devname(" is no device")));
            }
        }

        int flags = O_RDWR; /* required */
        if (nonblocking) { //nonblocking -> select
            flags |= O_NONBLOCK;
        }
        fd = open(dev_name.c_str(), flags, 0);
        if (fd == -1) {
            throw (camexc(dev_name + " open failed"));
        }

        if (show_inputs) {
            list_inputs(std::clog);
        }
        if (show_controls) {
            std::clog << "Supported Controls:\n";
            list_controls(std::clog);
        }

        v4l2_capability cap;

        if (xioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
            throw (camexc(debug_error("VIDIOC_QUERYCAP '" + dev_name + "'")));
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
            throw (camexc(debug_devname(" is no video capture device")));
        }

        switch (io) {
        case IO_METHOD::READ:
            if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
                throw (camexc(debug_devname(" does not support read i/o")));
                //exit (EXIT_FAILURE);
            }
            break;

        case IO_METHOD::MMAP:
        case IO_METHOD::USERPTR:
            if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
                throw (camexc(debug_devname(" does not support streaming i/o")));
                //exit (EXIT_FAILURE);
            }
            break;
        }

        if ((cap.capabilities & V4L2_CAP_VIDEO_OVERLAY)) {
            LOG(debug_devname(" is a video overlay device"));
        }

        v4l2_format fmt;
        if (pixelformat == V4L2_PIX_FMT_RGB565) {
            v4l2_framebuffer framebuffer;
            memset(&framebuffer, 0, sizeof(v4l2_framebuffer));
            if (xioctl(fd, VIDIOC_G_FBUF, &framebuffer) == -1) {
                LOG(debug_error("VIDIOC_G_FBUF failed"));
            }

            framebuffer.fmt.width = width;
            framebuffer.fmt.height = height;
            framebuffer.fmt.pixelformat = pixelformat;
            framebuffer.fmt.field = V4L2_FIELD_NONE;
            if (xioctl(fd, VIDIOC_S_FBUF, &framebuffer) == -1) {
                LOG(debug_error("VIDIOC_S_FBUF failed"));
            }
        }

        memset(&fmt, 0, sizeof(v4l2_format));
        fmt.type = type;
        if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt)) {
            throw (camexc(debug_error("VIDIOC_G_FMT '" + dev_name + "'")));
        }

        if (pixelformat == V4L2_PIX_FMT_RGB565) {
            fmt.fmt.win.w.height = height;
            fmt.fmt.win.w.width = width;
        }
        if (pixelformat == V4L2_PIX_FMT_YUYV) {
            fmt.fmt.pix.width = width;
            fmt.fmt.pix.height = height;
            fmt.fmt.pix.pixelformat = pixelformat;
            //fmt.fmt.pix.field = V4L2_FIELD_ANY;
            fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
        }

        if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt)) {
            throw (camexc(debug_error("VIDIOC_S_FMT '" + dev_name + "'")));
        }

        if (VERBOSE_CAMERA)
            std::clog
                    << "Height: " << fmt.fmt.pix.height << '\n'
                    << "Width:  " << fmt.fmt.pix.width << '\n'
                    << "pixfmt: " << static_cast<char>(fmt.fmt.pix.pixelformat>>0) <<
                    static_cast<char>(fmt.fmt.pix.pixelformat>>8) << static_cast<char>
                    (fmt.fmt.pix.pixelformat>>16) << static_cast<char>(fmt.fmt.pix.pixelformat>>24)
                    << '\n'
                    << "field:  " << fmt.fmt.pix.field  << '\n' << '\n';

        // set frame rate
        v4l2_streamparm fps;
        memset(&fps, 0, sizeof(v4l2_streamparm));
        fps.type = type;
        ///@todo replace with throw //@todo
        assert(ioctl(fd, VIDIOC_G_PARM, &fps) == 0);
        fps.parm.capture.timeperframe.numerator = 1;
        fps.parm.capture.timeperframe.denominator = camera::fps;
        ///@todo replace with throw //@todo
        assert(ioctl(fd, VIDIOC_S_PARM, &fps) != -1);

        // make sure automatic stuff is off
        // set defaults
        // request camera's default control settings
        // int exp, int bright, int g, int red, int blue, int cont, int sat
        // we set the values manually in VisionYUV::VisionYUV()

        set_parameter();

        switch (io) {
        case IO_METHOD::READ:
            init_read(fmt.fmt.pix.sizeimage);
            break;

        case IO_METHOD::MMAP:
            init_mmap();
            break;

        case IO_METHOD::USERPTR:
            init_userp(fmt.fmt.pix.sizeimage);
            break;
        }

        for (unsigned int i = 0; i < n_buffers; ++i) {
            v4l2_buffer buf;

            memset(&buf, 0, sizeof(v4l2_buffer));

            buf.type = type;
            buf.memory = memory;
            buf.index = i;
            if (-1 == xioctl(fd, VIDIOC_QBUF, &buf)) {
                throw (camexc(debug_error("VIDIOC_QBUF '" + dev_name + "'")));
            }
        }
        if (VERBOSE_CAMERA) {
            std::clog << '\n';
        }

    } catch (std::string &e) {
        std::cerr << e << std::endl;
        exit(EXIT_FAILURE);
    }
}

NaoCamera::Camera::~Camera() {
    // i'm not sure we're to place that.
    // normaly, our framework needs to ensure, that the camera stops
    // capturing.
    //stop_capturing();

    switch (io) {
    case IO_METHOD::READ:
        free(buffers[0].start);
        break;

    case IO_METHOD::MMAP:
        for (unsigned int i = 0; i < n_buffers; ++i)
            if (-1 == munmap(buffers[i].start, buffers[i].length)) {
                std::cerr << "warning, munmap!" << std::endl;
            }
        //throw(camexc(debug_error("munmap")));
        break;

    case IO_METHOD::USERPTR:
        for (unsigned int i = 0; i < n_buffers; ++i) {
            free(buffers[i].start);
        }
        break;
    }

    delete[](buffers);

    if (-1 == close(fd)) {
        std::cerr << debug_error("close") << std::endl;
    }
    fd = -1;
}

void NaoCamera::Camera::init_read(unsigned int buffer_size) {

    buffers = (buffer *)calloc(1, sizeof(*buffers));

    if (!buffers) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    buffers[0].length = buffer_size;
    buffers[0].start = malloc(buffer_size);

    if (!buffers[0].start) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
}

void NaoCamera::Camera::init_mmap(void) {
    // register buffers
    v4l2_requestbuffers req;
    memset(&req, 0, sizeof(v4l2_requestbuffers));
    req.count = n_buffers;
    req.type = type;
    req.memory = memory;

    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            throw (camexc(dev_name + " does not support memory mapping"));
            //exit(EXIT_FAILURE);
        } else {
            throw (camexc(debug_error("VIDIOC_REQBUFS '" + dev_name + "'")));
            //exit(EXIT_FAILURE);
        }
    }

    if (req.count < 1) {
        throw (camexc("Insufficient buffer memory on " + dev_name));
    }

    //  buffers = static_cast<buffer*>(calloc (req.count, sizeof (buffer)));
    buffers = new buffer[req.count];

    v4l2_buffer buf;
    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {

        memset(&buf, 0, sizeof(v4l2_buffer));

        buf.type = type;
        buf.memory = memory;
        buf.index = n_buffers;

        if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf)) {
            throw (camexc(debug_error("VIDIOC_QUERYBUF '" + dev_name + "'")));
        }

        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start =
            mmap(NULL /* start anywhere */,
                 buf.length,
                 PROT_READ | PROT_WRITE /* required */,
                 MAP_SHARED /* recommended */,
                 fd, buf.m.offset);

        if (static_cast<buffer *>(MAP_FAILED) == buffers[n_buffers].start) {
            throw (camexc(debug_error("mmap '" + dev_name + "'")));
        }

        LOG_DEBUG << "Created buffer " << n_buffers << " at "
                  << buffers[n_buffers].start << " with length "
                  << static_cast<unsigned long>(buffers[n_buffers].length)
                  << ".";

    }
}

void NaoCamera::Camera::init_userp(unsigned int buffer_size) {
    v4l2_requestbuffers req;
    unsigned int page_size;

    page_size = getpagesize();
    buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);

    // 	CLEAR (req);

    req.count               = 4;
    req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory              = V4L2_MEMORY_USERPTR;

    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            throw (camexc(debug_devname(" does not support user pointer i/o")));
            //exit (EXIT_FAILURE);
        } else {
            std::cerr << debug_error("VIDIOC_REQBUFS") << std::endl;
        }
    }

    buffers = (buffer *)calloc(4, sizeof(*buffers));

    if (!buffers) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    for (n_buffers = 0; n_buffers < 4; ++n_buffers) {
        buffers[n_buffers].length = buffer_size;
        buffers[n_buffers].start = memalign(/* boundary */ page_size,
                                   buffer_size);

        if (!buffers[n_buffers].start) {
            fprintf(stderr, "Out of memory\n");
            exit(EXIT_FAILURE);
        }
    }
}

void *NaoCamera::Camera::start(const v4l2_buffer *const buf) const {
    switch (io) {
    case IO_METHOD::READ:
        return buffers[0].start;
    case IO_METHOD::MMAP:
        return buffers[buf->index].start;
    case IO_METHOD::USERPTR:
        return (void *) buf->m.userptr;
    }
    JS_UNREACHABLE();
}

std::size_t NaoCamera::Camera::length(const v4l2_buffer *const buf) const {
    switch (io) {
    case IO_METHOD::READ:
        return buffers[0].length;
    case IO_METHOD::MMAP:
        return buffers[buf->index].length;
    case IO_METHOD::USERPTR:
        return 0;//(void *) buf->m.userptr;
    }
    JS_UNREACHABLE();
}


/**
 * toggle the camera
 * @return true on success
 */
bool NaoCamera::Camera::ToggleCamera() {
    std::cerr << "toggle camera disabled!" << std::endl;
    return true;
}

/**
 * @param id identifies the control
 * @return true on success
 */
int NaoCamera::Camera::getControl(const int &id) const {
    v4l2_queryctrl queryctrl;
    memset(&queryctrl, 0, sizeof(queryctrl));
    queryctrl.id = id;
    if (xioctl(fd, VIDIOC_QUERYCTRL, &queryctrl) < 0) {
        return -1;
    }
    if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
        return -1;    // not available
    }
    if (queryctrl.type != V4L2_CTRL_TYPE_BOOLEAN
            && queryctrl.type != V4L2_CTRL_TYPE_INTEGER
            && queryctrl.type != V4L2_CTRL_TYPE_MENU) {
        return -1;    // not supported
    }

    v4l2_control control_s;
    memset(&control_s, 0, sizeof(control_s));
    control_s.id = id;
    if (xioctl(fd, VIDIOC_G_CTRL, &control_s) < 0) {
        return -1;
    }
    // if(control_s.value == queryctrl.default_value)
    // 	return -1;
    return control_s.value;
}

/**
* @param id identifies the control
* @param value New value value.
* @return true on success
*/
bool NaoCamera::Camera::setControl(const int &id, const int &value) {

    int temp = value;
    return setControl(id, temp);

}

/**
 * @param id identifies the control
 * @param value New value value.
 * @return true on success
 */
bool NaoCamera::Camera::setControl(const int &id, int &value) {
    v4l2_queryctrl queryctrl;
    memset(&queryctrl, 0, sizeof(queryctrl));
    queryctrl.id = id;
    if (xioctl(fd, VIDIOC_QUERYCTRL, &queryctrl) < 0) {
        return false;
    }
    if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
        return false;    // not available
    }
    if (queryctrl.type != V4L2_CTRL_TYPE_BOOLEAN
            && queryctrl.type != V4L2_CTRL_TYPE_INTEGER
            && queryctrl.type != V4L2_CTRL_TYPE_MENU) {
        return false;    // not supported
    }

    v4l2_control control_s;
    memset(&control_s, 0, sizeof(control_s));
    control_s.id = id;
    if (value > queryctrl.maximum) {
        control_s.value = queryctrl.maximum;
    } else {
        if (value < queryctrl.minimum) {
            control_s.value = queryctrl.minimum;
        } else {
            control_s.value = value;
        }
    }

    if (xioctl(fd, VIDIOC_S_CTRL, &control_s) < 0) {
        return false;
    }

    value = control_s.value;
    return true;
}

/**
 * turn the stream on \n
 * or activate the overlay
 * @return true on success
 */
bool NaoCamera::Camera::start_capturing() const {
    switch (io) {
    case IO_METHOD::READ:
        /* Nothing to do. */
        break;

    case IO_METHOD::MMAP:
    case IO_METHOD::USERPTR: {
        if (pixelformat == V4L2_PIX_FMT_YUYV) { // start streaming
            if (-1 == xioctl(fd, VIDIOC_STREAMON, &type)) {
                std::cerr << debug_error("VIDIOC_STREAMON '" + dev_name + "'") << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        if (pixelformat == V4L2_PIX_FMT_RGB565) {
            int state = 1;
            if (-1 == xioctl(fd, VIDIOC_OVERLAY, &state)) {
                std::cerr << debug_error("VIDIOC_OVERLAY '" + dev_name + "'") << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        break;
    }
    }
    return true;
}

/**
 * turn the stream off \n
 * or deactivate the overlay
 * @return true on success
 */
bool NaoCamera::Camera::stop_capturing() const {
    switch (io) {
    case IO_METHOD::READ:
        /* Nothing to do. */
        break;

    case IO_METHOD::MMAP:
    case IO_METHOD::USERPTR: {
        if (pixelformat == V4L2_PIX_FMT_YUYV) {
            if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type)) {
                std::cerr << debug_error("VIDIOC_STREAMOFF") << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        if (pixelformat == V4L2_PIX_FMT_RGB565) {
            int state = 0;
            if (-1 == xioctl(fd, VIDIOC_OVERLAY, &state)) {
                std::cerr << debug_error("VIDIOC_OVERLAY '" + dev_name + "'") << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        break;
    }
    }
    return true;
}

void NaoCamera::Camera::list_inputs(std::ostream &out) const {
    v4l2_input input;
    int index;

    if (-1 == ioctl(fd, VIDIOC_G_INPUT, &index)) {
        std::cerr << "VIDIOC_G_INPUT" << std::endl;
        exit(EXIT_FAILURE);
    }

    memset(&input, 0, sizeof(input));
    input.index = index;

    if (-1 == ioctl(fd, VIDIOC_ENUMINPUT, &input)) {
        std::cerr << "VIDIOC_ENUMINPUT" << std::endl;
        exit(EXIT_FAILURE);
    }

    out << "Current input: " << input.name << '\n'
        << "type: " << input.type <<  '\n'
        << "audioset: " << input.audioset << '\n'
        << "tuner: " << input.tuner << '\n'
        << "std: " << input.std << '\n'
        << "status: " << input.status << '\n' << '\n';

    /*memset (&input, 0, sizeof (input));
    input.index = index+1;

    if (-1 == ioctl (fd, VIDIOC_ENUMINPUT, &input) && errno != EINVAL) {
      std::cerr << "VIDIOC_ENUMINPUT" << std::endl;
    }
    if(errno != EINVAL){
      out << "Current input: " << input.name << '\n'
      << "type: " << input.type <<  '\n'
      << "audioset: " << input.audioset << '\n'
      << "tuner: " << input.tuner << '\n'
      << "std: " << input.std << '\n'
      << "status: " << input.status << '\n'<< '\n';
    }*/
}

void NaoCamera::Camera::set_parameter() {

    /*setControl(V4L2_CID_BACKLIGHT_COMPENSATION, 1);
    std::clog << "V4L2_CID_BACKLIGHT_COMPENSATION ("
              << V4L2_CID_BACKLIGHT_COMPENSATION
              << ") set: 1, read: "
              << getControl(V4L2_CID_BACKLIGHT_COMPENSATION) << '\n';
    usleep(100000);
    setControl(V4L2_CID_EXPOSURE_AUTO, 1);
    std::clog << "V4L2_CID_EXPOSURE_AUTO ("
              << V4L2_CID_EXPOSURE_AUTO
              << ") set: 1, read: "
              << getControl(V4L2_CID_EXPOSURE_AUTO) << '\n';
    usleep(100000);
    setControl(V4L2_CID_AUTO_WHITE_BALANCE, 1);
    std::clog << "V4L2_CID_AUTO_WHITE_BALANCE ("
              << V4L2_CID_AUTO_WHITE_BALANCE
              << ") set: 1, read: "
              << getControl(V4L2_CID_AUTO_WHITE_BALANCE) << '\n';
    usleep(100000);*/

    /*bool success = false;
    int newValue;
    // first set brightness
    while (!success) {
        newValue = settings.at(0).brightness;
        if (newValue < 0) newValue = 0;
        if (newValue > 255) newValue = 255;
        setControl(V4L2_CID_BRIGHTNESS, newValue);
        std::clog << "V4L2_CID_BRIGHTNESS ("
        << V4L2_CID_BRIGHTNESS
        << ") set: " << newValue
        << ", read: " << getControl(V4L2_CID_BRIGHTNESS) << '\n';
        success = (abs(getControl(V4L2_CID_BRIGHTNESS) - newValue) < 5);
    }

    setControl(V4L2_CID_BACKLIGHT_COMPENSATION, 0);
    std::clog << "V4L2_CID_BACKLIGHT_COMPENSATION ("
    << V4L2_CID_BACKLIGHT_COMPENSATION
    << ") set: 0, read: "
    << getControl(V4L2_CID_BACKLIGHT_COMPENSATION) << '\n';

    setControl(V4L2_CID_EXPOSURE_AUTO, 0);
    std::clog << "V4L2_CID_EXPOSURE_AUTO ("
    << V4L2_CID_EXPOSURE_AUTO
    << ") set: 0, read: "
    << getControl(V4L2_CID_EXPOSURE_AUTO) << '\n';

    setControl(V4L2_CID_AUTO_WHITE_BALANCE, 0);
    std::clog << "V4L2_CID_AUTO_WHITE_BALANCE ("
    << V4L2_CID_AUTO_WHITE_BALANCE
    << ") set: 0, read: "
    << getControl(V4L2_CID_AUTO_WHITE_BALANCE) << '\n';

    setControl(V4L2_CID_BRIGHTNESS, settings.at(0).brightness);
    std::clog << "V4L2_CID_BRIGHTNESS ("
    << V4L2_CID_BRIGHTNESS
    << ") set: " << settings.at(0).brightness
    << ", read: " << getControl(V4L2_CID_BRIGHTNESS) << '\n';

    setControl(V4L2_CID_CONTRAST, settings.at(0).contrast);
    std::clog << "V4L2_CID_CONTRAST ("
    << V4L2_CID_CONTRAST
    << ") set: " << settings.at(0).contrast
    << ", read: " << getControl(V4L2_CID_CONTRAST) << '\n';

    setControl(V4L2_CID_SATURATION, settings.at(0).saturation);
    std::clog << "V4L2_CID_SATURATION ("
    << V4L2_CID_SATURATION
    << ") set: " << settings.at(0).saturation
    << ", read: " << getControl(V4L2_CID_SATURATION) << '\n';

    setControl(V4L2_CID_EXPOSURE, settings.at(0).exposure);
    std::clog << "V4L2_CID_EXPOSURE ("
    << V4L2_CID_EXPOSURE
    << ") set: " << settings.at(0).exposure
    << ", read: " << getControl(V4L2_CID_EXPOSURE) << '\n';

    setControl(V4L2_CID_GAIN, settings.at(0).gain);
    std::clog << "V4L2_CID_GAIN ("
    << V4L2_CID_GAIN
    << ") set: " << settings.at(0).gain
    << ", read: " << getControl(V4L2_CID_GAIN) << '\n';

    setControl(V4L2_CID_SHARPNESS, settings.at(0).sharpness);
    std::clog << "V4L2_CID_SHARPNESS ("
    << V4L2_CID_SHARPNESS
    << ") set: " << settings.at(0).sharpness
    << ", read:" << getControl(V4L2_CID_SHARPNESS) << '\n';

    success = false;
    int errorCnt = 0;
    while (!success && (errorCnt < 30)) {
        newValue = settings.at(0).whiteBalance;
        if (newValue < -120) newValue = -120;
        if (newValue > -36) newValue = -36;
        setControl(V4L2_CID_DO_WHITE_BALANCE, newValue); // DO WHITE BALANCE
        std::clog << "V4L2_CID_DO_WHITE_BALANCE ("
        << V4L2_CID_DO_WHITE_BALANCE
        << ") set: " << newValue
        << ", read: " << getControl(V4L2_CID_DO_WHITE_BALANCE) << '\n';
        success = (getControl(V4L2_CID_DO_WHITE_BALANCE) == newValue);
        ++errorCnt;
    }
    if (errorCnt >= 30) {
        std::cerr << "++++++++++++++++++++++++" << std::endl;
        std::cerr << "ATTENTION:" << std::endl;
        std::cerr << "I was not able to set the White Balance Value for the camera!" << std::endl;
        std::cerr << "this could lead to massive problems!" << std::endl;
        std::cerr << "++++++++++++++++++++++++" << std::endl;
    }*/

    if (activeCam == 0) {
        setControl(V4L2_CID_HFLIP, 1);
        LOG_DEBUG << "V4L2_CID_HFLIP set: 1, read: " << getControl(V4L2_CID_HFLIP) ;
        setControl(V4L2_CID_VFLIP, 1);
        LOG_DEBUG << "V4L2_CID_VFLIP set: 1, read: " << getControl(V4L2_CID_VFLIP) ;
    } else {
        setControl(V4L2_CID_HFLIP, 0);
        LOG_DEBUG << "V4L2_CID_HFLIP set: 0, read: " << getControl(V4L2_CID_HFLIP) ;
        setControl(V4L2_CID_VFLIP, 0);
        LOG_DEBUG << "V4L2_CID_VFLIP set: 0, read: " << getControl(V4L2_CID_VFLIP) ;
    }

    // disable auto stuff! temporary. use config file
    setControl(10094849, 0);
    LOG_DEBUG << "DISABLE AUTO EXPOSURE and read it again: " << getControl(
                  10094849);
    setControl(9963788, 0);
    LOG_DEBUG << "DISABLE AUTO WHITE BALANCE and read it again: " << getControl(
                  9963788);
    setControl(134217728, 0);
    LOG_DEBUG << "DISABLE FADE TO BLACK and read it again: " << getControl(
                  134217728);

}

void NaoCamera::Camera::list_controls(std::ostream &out) const {
    /// @todo     http://v4l2spec.bytesex.org/spec/x802.htm
    /// @todo     ivtv v4l2-ctl source

    v4l2_queryctrl queryctrl;
    queryctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
    int id;

    while (ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl) == 0) {
        print_control(out, queryctrl);
        queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
    }
    if (queryctrl.id != V4L2_CTRL_FLAG_NEXT_CTRL) {
        return;
    }
    for (id = V4L2_CID_USER_BASE; id < V4L2_CID_LASTP1; id++) {
        queryctrl.id = id;
        if (ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl) == 0) {
            print_control(out, queryctrl);
        }
    }


    for (queryctrl.id = V4L2_CID_PRIVATE_BASE;
            ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl) == 0; ++queryctrl.id) {
        //     out << "Control " << queryctrl.name << '\n';
        print_control(out, queryctrl);
    }
}




void NaoCamera::Camera::enumerate_menu(std::ostream &out,
                                       const v4l2_queryctrl &queryctrl) const {
    ///@todo keep menus?
    v4l2_querymenu querymenu;
    querymenu.index = 0;

    out << " (";
    querymenu.id = queryctrl.id;

    for (querymenu.index = queryctrl.minimum;
            querymenu.index <= static_cast<unsigned int>(queryctrl.maximum);
            ++querymenu.index) {
        int ret = 0;
        ret = ioctl(fd, VIDIOC_QUERYMENU, &querymenu);
        if (ret == 0) {
            ///@todo       out << '(' << querymenu.name << ',' << querymenu.index << ')';
            out << querymenu.name;
        } else {
            //       std::cerr << "VIDIOC_QUERYMENU" << errno << strerror(errno) << std::endl;
            //       __asm__ __volatile__ ("int3");
            //       exit (EXIT_FAILURE);
            //       --querymenu.index;
            //       continue;
            std::cerr << "control not found" << std::endl;
        }
        if (querymenu.index < static_cast<unsigned int>(queryctrl.maximum)) {
            out << ',';
        } else {
            out << ')';
        }
    }
}

typedef struct {
    unsigned flag;
    const char *str;
} flag_def;


static std::string name2var(const unsigned char *name) {
    std::string s;
    int add_underscore = 0;

    while (*name) {
        if (isalnum(*name)) {
            if (add_underscore) {
                s += '_';
            }
            add_underscore = 0;
            s += std::string(1, tolower(*name));
        } else {
            if (s.length()) {
                add_underscore = 1;
            }
        }
        name++;
    }
    return s;
}

static std::string flags2s(unsigned val, const flag_def *def) {
    std::string s;

    while (def->flag) {
        if (val & def->flag) {
            if (s.length()) {
                s += " ";
            }
            s += def->str;
        }
        def++;
    }
    return s;
}

static void print_qctrl(int fd, const v4l2_queryctrl *queryctrl,
                        v4l2_ext_control *ctrl, int show_menus) {
    v4l2_querymenu qmenu;
    std::string s = name2var(queryctrl->name);

    qmenu.id = queryctrl->id;
    switch (queryctrl->type) {
    case V4L2_CTRL_TYPE_INTEGER:
        printf("%31s (int)  : min=%d max=%d step=%d default=%d value=%d",
               s.c_str(),
               queryctrl->minimum, queryctrl->maximum,
               queryctrl->step, queryctrl->default_value,
               ctrl->value);
        break;
    case V4L2_CTRL_TYPE_INTEGER64:
        printf("%31s (int64): value=%lld", s.c_str(), ctrl->value64);
        break;
    case V4L2_CTRL_TYPE_BOOLEAN:
        printf("%31s (bool) : default=%d value=%d",
               s.c_str(),
               queryctrl->default_value, ctrl->value);
        break;
    case V4L2_CTRL_TYPE_MENU:
        printf("%31s (menu) : min=%d max=%d default=%d value=%d",
               s.c_str(),
               queryctrl->minimum, queryctrl->maximum,
               queryctrl->default_value, ctrl->value);
        break;
    case V4L2_CTRL_TYPE_BUTTON:
        printf("%31s (button)\n", s.c_str());
        break;
    default:
        break;
    }
    if (queryctrl->flags) {
        const flag_def def[] = {
            { V4L2_CTRL_FLAG_GRABBED,    "grabbed" },
            { V4L2_CTRL_FLAG_READ_ONLY,  "read-only" },
            { V4L2_CTRL_FLAG_UPDATE,     "update" },
            { V4L2_CTRL_FLAG_INACTIVE,   "inactive" },
            { V4L2_CTRL_FLAG_SLIDER,     "slider" },
            { V4L2_CTRL_FLAG_WRITE_ONLY, "write-only" },
            { 0, NULL }
        };
        printf(" flags=%s", flags2s(queryctrl->flags, def).c_str());
    }
    printf("\n");
    if (queryctrl->type == V4L2_CTRL_TYPE_MENU && show_menus) {
        for (int i = 0; i <= queryctrl->maximum; i++) {
            qmenu.index = i;
            if (ioctl(fd, VIDIOC_QUERYMENU, &qmenu)) {
                continue;
            }
            printf("\t\t\t\t%d: %s\n", i, qmenu.name);
        }
    }
}


void NaoCamera::Camera::print_control(std::ostream &out,
                                      const v4l2_queryctrl &queryctrl) const {
    v4l2_control ctrl;
    v4l2_ext_control ext_ctrl;
    v4l2_ext_controls ctrls;

    if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
        return;
    }
    if (queryctrl.type == V4L2_CTRL_TYPE_CTRL_CLASS) {
        printf("\n%s\n\n", queryctrl.name);
        return;
    }
    ext_ctrl.id = queryctrl.id;
    ctrls.ctrl_class = V4L2_CTRL_ID2CLASS(queryctrl.id);
    ctrls.count = 1;
    ctrls.controls = &ext_ctrl;
    if (V4L2_CTRL_ID2CLASS(queryctrl.id) != V4L2_CTRL_CLASS_USER &&
            queryctrl.id < V4L2_CID_PRIVATE_BASE) {
        if (ioctl(fd, VIDIOC_G_EXT_CTRLS, &ctrls)) {
            printf("error %d getting ext_ctrl %s\n",
                   errno, queryctrl.name);
            return;
        }
    } else {
        ctrl.id = queryctrl.id;
        if (ioctl(fd, VIDIOC_G_CTRL, &ctrl)) {
            printf("error %d getting ctrl %s\n",
                   errno, queryctrl.name);
            return;
        }
        ext_ctrl.value = ctrl.value;
    }
    print_qctrl(fd, &queryctrl, &ext_ctrl, true);
    return;

}

#if 0
namespace std {
string to_string(long long int i) {
    std::ostringstream buf;
    buf << i;
    return buf.str();
}
string to_string(long int i) {
    return to_string(static_cast<long long int>(i));
}
string to_string(int i) {
    return to_string(static_cast<long long int>(i));
}
}
#endif


namespace NaoCamera {
std::ostream &operator<<(std::ostream &out, const Settings &settings) {
    return out
           << "-> exposure: " << settings.exposure << '\n'
           << "-> brightness: " << settings.brightness << '\n'
           << "-> gain: " << settings.gain << '\n'
           << "-> contrast: " << settings.contrast << '\n'
           << "-> saturation: " << settings.saturation << '\n';
}

std::ostream &operator<<(std::ostream &out,
                         v4l2_buffer const *const v4l2_buf) {
    return out
           << "bytesused: "  << v4l2_buf->bytesused << '\n'
           << "flags: "      << v4l2_buf->flags << '\n'
           << "field: "      << v4l2_buf->field << '\n'
           << "timestamp: "  << v4l2_buf->timestamp.tv_sec << "." <<
           v4l2_buf->timestamp.tv_usec << '\n'
           << "sequence: "   << v4l2_buf->sequence << '\n';
}
}


// vim: set ts=4 sw=4 sts=4 expandtab:
