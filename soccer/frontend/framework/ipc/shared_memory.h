#pragma once

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

namespace ipc {

template<typename T>
class SharedMemory {

public:
    SharedMemory() = default;

    SharedMemory(const std::string &name, bool create) : 
            myName(name),
            createdShm(create) {

        using namespace boost::interprocess;

        if (createdShm) {
            shared_memory_object::remove(myName.c_str());
            shm = shared_memory_object(create_only, myName.c_str(), read_write);  
            shm.truncate(sizeof(T));
        } else {
            shm = shared_memory_object(open_only, myName.c_str(), read_write);
        }

        map = mapped_region(shm, read_write);

        if (createdShm) {
            void *addr = map.get_address();
            data = new (addr) T;
        } else {
            data = static_cast<T *>(map.get_address());
        }
    }

    ~SharedMemory() {
        using namespace boost::interprocess;
        if (createdShm) {
            shared_memory_object::remove(myName.c_str());
        }
    }

    SharedMemory(const SharedMemory&) = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;

    T *operator->() {
        return data;
    }

    T &operator*() {
        return *data;
    }

    std::string name() const { return myName; }


private:
    std::string myName = "";
    bool createdShm = false;

    boost::interprocess::shared_memory_object shm;
    boost::interprocess::mapped_region map;

    T *data = nullptr;
};

} // namespace ipc
