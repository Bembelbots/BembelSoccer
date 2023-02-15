#pragma once

#include <string>
#include <vector>
#include <fstream>

// Abstract LoggerBackend to derive new backends from
class BaseLoggerBackend {
public:
    std::string id;
    std::string name;

    BaseLoggerBackend(const std::string &my_id, const std::string &my_name);
    virtual ~BaseLoggerBackend();

    virtual void write(const std::string &text) = 0;
    virtual void init();
    virtual void cleanup();
};

// append log data to a file named 'fn'
// re-open it for every write
class FileBackend : public BaseLoggerBackend {
public:
    FileBackend(const std::string &my_id, const std::string &fn, const std::string &my_name = "file-append");
    virtual ~FileBackend();

    void write(const std::string &msg) override;

protected:
    virtual void write_to_fstream(std::ofstream &fd, const std::string &msg);

    std::string filename;
};

// the only difference to FileBackend: keep open file-descriptor!
class PersistentFileBackend : public FileBackend {
public:
    PersistentFileBackend(
            const std::string &my_id, const std::string &fn, const std::string &my_name = "persistent-file-append");
    virtual ~PersistentFileBackend();

    void write(const std::string &msg) override;

protected:
    void init() override;
    void cleanup() override;

    std::ofstream fd;
};

// Persistent _AND_ Buffered (likely the best performing log-file-backend)
class BufferedFileBackend : public PersistentFileBackend {
public:
    BufferedFileBackend(const std::string &my_id, const std::string &fn, const int &buf_size,
            const std::string &my_name = "buffered-pers-file-append");
    virtual ~BufferedFileBackend();

    void write(const std::string &msg) override;

    // (force) flush buffer and write it to file!
    void flush();

protected:
    // flush with cleanup!
    void cleanup() override;

    std::string _buf;
    size_t _buffer_size;
};

// class MutexedFileBackend (not necassary I think...)
// class UDP/TCPServerBackend

// cout-based log-to-console backend
class ConsoleBackend : public BaseLoggerBackend {
public:
    explicit ConsoleBackend(const std::string &my_id);
    void write(const std::string &msg) override;
};

// Simple keep-in-memory backend
class MemoryBackend : public BaseLoggerBackend {
public:
    std::vector<std::string> log_msgs;

    explicit MemoryBackend(const std::string &my_id);
    void write(const std::string &msg) override;
};
