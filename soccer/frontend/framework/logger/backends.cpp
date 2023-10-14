#include <iostream>
#include <fstream>

#include "backends.h"

// BaseLoggerBackend abstract class
BaseLoggerBackend::BaseLoggerBackend(const std::string &my_id, const std::string &my_name) : id(my_id), name(my_name) {
    init();
}

BaseLoggerBackend::~BaseLoggerBackend() {
    cleanup();
}

// Hook for custom initialization
void BaseLoggerBackend::init() {
}

// Hook for custom destructor/cleanup
void BaseLoggerBackend::cleanup() {
}

// FileBackend realization
FileBackend::FileBackend(const std::string &my_id, const std::string &fn, const std::string &my_name)
  : BaseLoggerBackend(my_id, my_name), filename(fn) {
}

FileBackend::~FileBackend() {
}

// writing to file: opening - appending message - closing
void FileBackend::write(const std::string &msg) {
    std::ofstream fd(filename.c_str(), std::ios::app);
    write_to_fstream(fd, msg);
    fd.close();
}

// internal, write 'msg' to 'fd'
void FileBackend::write_to_fstream(std::ofstream &fd, const std::string &msg) {
    fd.write(msg.c_str(), msg.size());
}

// PersistentFileBackend keeps the fileobject open
PersistentFileBackend::PersistentFileBackend(
        const std::string &my_id, const std::string &fn, const std::string &my_name)
  : FileBackend(my_id, fn, my_name) {
}

PersistentFileBackend::~PersistentFileBackend() {
}

// write directly to file
void PersistentFileBackend::write(const std::string &msg) {
    write_to_fstream(fd, msg);
}

// open file-descriptor wrapper
void PersistentFileBackend::init() {
    fd.open(filename.c_str(), std::ios::app);
}

// close file-descriptor wrapper
void PersistentFileBackend::cleanup() {
    fd.close();
}

// BufferedFileBackend
BufferedFileBackend::BufferedFileBackend(
        const std::string &my_id, const std::string &fn, const int &buf_size, const std::string &my_name)
  : PersistentFileBackend(my_id, fn, my_name), _buffer_size(buf_size) {
}

BufferedFileBackend::~BufferedFileBackend() {
}

// first copy data to buffer, once it overflows, flush into file!
void BufferedFileBackend::write(const std::string &msg) {
    if (_buf.size() + msg.size() > _buffer_size)
        flush();

    _buf.append(msg);
}

// flush buffer contents and write them to file
void BufferedFileBackend::flush() {
    write_to_fstream(fd, _buf);
    fd.flush();
    _buf.clear();
}

void BufferedFileBackend::cleanup() {
    flush();
    fd.close();
}

// ConsoleBackend realization
ConsoleBackend::ConsoleBackend(const std::string &my_id) : BaseLoggerBackend(my_id, "stdout") {
}

// write to stdout through "cout"
void ConsoleBackend::write(const std::string &msg) {
    std::cout << msg << std::flush;
}

// MemoryBackend
MemoryBackend::MemoryBackend(const std::string &my_id) : BaseLoggerBackend(my_id, "memory") {
    log_msgs.clear();
}

// save data to logger vector
void MemoryBackend::write(const std::string &msg) {
    log_msgs.push_back(msg);
}
