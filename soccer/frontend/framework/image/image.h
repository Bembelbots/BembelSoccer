#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/core/core_c.h>
#include <atomic>
#include <bitset>
#include <memory>
#include <cstdint>
#include <fstream>

class Pixel {};

enum class ImgLock {
    CAPTURE,
    VISION,
    DEBUG,
    WRITER
};

class ImageBase {
public:
    size_t width;
    size_t height;
    uint8_t *data;
    int64_t timestamp;

    // returns true, if an image was set
    bool hasData();

    // write image to already opened file stream
    virtual void write(std::ofstream &file);

    // write image to given filename destination
    virtual void saveAs(std::string path);

    // returns cv mat of image
    // \todo: is this always set?
    cv::Mat mat() const;

    // resize image to given size.
    void resize(size_t w, size_t h) __attribute__ ((deprecated));

    // check im image has a valid data pointer
    bool isValid();

    // checks if a given position is inside image boundaries.
    bool isInside(size_t w, size_t h) const;

    // set image data
    void setData(uint8_t *img);
    void setData(char *data);

    bool isLocked() const;
    bool hasLock(const ImgLock &lt) const;
    void lock(const ImgLock &lt);
    void unlock(const ImgLock &lt);

protected:
    using ImgLocks_t = std::atomic<std::bitset<8>>;
    std::shared_ptr<ImgLocks_t> locks;
    std::shared_ptr<uint8_t> p_container;
    int channels;
    int cvType;

    // default constructors
    ImageBase();
    ImageBase(uint8_t *img, size_t width, size_t height, int channels, int cvType);
    ImageBase(size_t width, size_t height, int channels, int cvType);
    // we neeeed a destrcutor
    virtual ~ImageBase();

    // default destructor for shared pointer instances.
    // this is called, if no one else uses the image data anymore
    static void deleteImagePointer(uint8_t *img);

    // getters for included ipl image data
    IplImage *getIplHeader() const;
    IplImage *getIplHeader(size_t w, size_t h) const;
    IplImage *getIplImage() const;
};

#define LOOP_IMAGE(img, step) \
    { \
        auto i = img; \
        for (size_t x = 0; x < i.width; x += step) { \
            for (size_t y = 0; y < i.height; y += step) {

#define LOOP_IMAGE_END(img) }}}


// vim: set ts=4 sw=4 sts=4 expandtab:
