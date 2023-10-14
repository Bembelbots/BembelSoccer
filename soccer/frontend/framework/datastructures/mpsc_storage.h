#include <vector>
#include <mutex>

template<typename T>
class MPSCStorage {
public:
    MPSCStorage() {}

    MPSCStorage(MPSCStorage<T> const& other) {
        std::scoped_lock<std::mutex> lk(mtx);
        storage = other.storage;
        offset = other.offset;
        invalid = other.invalid;
    }

    MPSCStorage<T> & operator=(MPSCStorage<T> const& other) {
        std::scoped_lock<std::mutex> lk(mtx);
        storage = other.storage;
        offset = other.offset;
        invalid = other.invalid;
        return *this;
    }

    T& aquire() {
        std::scoped_lock<std::mutex> lk(mtx);
        if(invalid) {
            storage = std::vector<T>();
        }
        storage.resize(++offset);
        return storage.back();
    }

    std::vector<T> take() {
        std::scoped_lock<std::mutex> lk(mtx);
        invalid = true;
        return std::move(storage);
    }

    void clear() {
        std::scoped_lock<std::mutex> lk(mtx);
        storage.clear();
    }

private:
    std::mutex mtx;
    std::vector<T> storage;
    int offset = 0;
    bool invalid = false;
};