#pragma once

#include <memory>
#include <string>
#include <vector>
#include <boost/asio/buffer.hpp>

// A reference-counted non-modifiable buffer class.
// shamelessly stolen from boost example:
// https://www.boost.org/doc/libs/1_73_0/doc/html/boost_asio/example/cpp11/buffers/reference_counted.cpp

class shared_const_buffer {
public:
    // Construct from a std::string.
    explicit shared_const_buffer(const std::string &data)
      : data_(new std::vector<char>(data.begin(), data.end())), buffer_(boost::asio::buffer(*data_)) {}

    // Construct from data pointer & size
    explicit shared_const_buffer(const char *data, size_t size)
      : shared_const_buffer(std::string(data, size)) {}

    // Implement the ConstBufferSequence requirements.
    typedef boost::asio::const_buffer value_type;
    typedef const boost::asio::const_buffer *const_iterator;
    const boost::asio::const_buffer *begin() const { return &buffer_; }
    const boost::asio::const_buffer *end() const { return &buffer_ + 1; }

private:
    std::shared_ptr<std::vector<char>> data_;
    boost::asio::const_buffer buffer_;
};
