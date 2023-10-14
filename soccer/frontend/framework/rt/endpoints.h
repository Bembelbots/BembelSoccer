#pragma once

#include "channels.h"
#include "../util/assert.h"

namespace rt {

class Linker;

template<typename T>
class Endpoint {

public:
    using QualifiedType = T;
    using PureType = typename std::remove_const_t<T>;

    QualifiedType &get() {
        jsassert(data != nullptr);
        return *data;
    }

    const PureType &get() const {
        jsassert(data != nullptr);
        return *data;
    }

    inline QualifiedType &operator*() { return get(); }
    inline const PureType &operator*() const { return get(); }

    inline QualifiedType *operator->() { return data; }
    inline const PureType *operator->() const { return data; }

    inline operator QualifiedType &() { return get(); }
    inline operator const PureType &() const { return get(); }

    inline operator QualifiedType *() { return data; }
    inline operator const PureType *() const { return data; }

protected:
    friend class Linker;
    PureType *data = nullptr;
};

struct Listen {};
struct Require {};
struct Snoop {};
struct Read {};
struct Write {};
struct Issue {};
struct Handle {};
struct Batch {};
struct Event {};
struct Default {};

template<typename T, typename Level = Listen>
class Input {

public:
    Input() = default;

    inline const T &operator*() const { return assure(); }
    inline const T *operator->() const { return &assure(); }
    inline operator const T &() const { return assure(); }
    inline operator const T *() const { return &assure(); }

private:
    friend class Linker;

    T data = {};
    int id = -1;

    inline bool connected() const { return id != -1; }

    const T &assure() const {
        jsassert(connected());
        return data;
    }
};

template<typename T>
class Input<T, Snoop> {

public:
    std::vector<T> fetch() { return assertLink().snoopFetch(id); }
    void waitWhileEmpty() { assertLink().waitWhileEmpty(id); }

private:
    friend class Linker;

    int id = -1;
    MessageChannel<T> *link = nullptr;

    auto &assertLink() {
        jsassert(link != nullptr && id != -1);
        return *link;
    }
};

template<typename T>
class Input<T, Event> : public Input<T, Snoop> {};

template<typename T, typename Level = Default>
class Output {

public:
    Output() = default;

    inline T &operator*() { return assure(); }
    inline T *operator->() { return &assure(); }
    inline operator T &() { return assure(); }
    inline operator T *() { return &assure(); }

private:
    friend class Linker;

    T data = {};
    bool connected = false;

    T &assure() {
        jsassert(connected);
        return data;
    }
};

template<typename T>
class Output<T, Batch> {

public:
    Output() = default;

    inline T &operator*() { return assure(); }
    inline T *operator->() { return &assure(); }
    inline operator T &() { return assure(); }
    inline operator T *() { return &assure(); }

    void send() { assertLink().write(assure()); }

private:
    friend class Linker;
    
    MessageChannel<T> *link = nullptr;

    T data = {};
    bool connected = false;

    T &assure() {
        jsassert(connected);
        return data;
    }
    
    auto &assertLink() {
        jsassert(link != nullptr);
        return *link;
    }
};

template<typename T>
class Output<T, Event> {
public:
    Output() = default;

    void emit(T &data) { assertLink().write(data); }
    void emit(T &&data) { assertLink().write(data); }

private:
    friend class Linker;
    
    MessageChannel<T> *link = nullptr;
 
    auto &assertLink() {
        jsassert(link != nullptr);
        return *link;
    }
};

template<typename T, typename Access = Read>
struct Context {};

template<typename T>
struct Context<T, Read> : public Endpoint<const T> {};

template<typename T>
struct Context<T, Write> : public Endpoint<T> {};

template<typename Group, typename Side = Issue>
struct Dispatch {};

template<typename Group>
struct Dispatch<Group, Issue> {

public:
    template<typename Command, typename = typename std::enable_if_t<is_command_v<Command, Group>>, typename... Args>
    void enqueue(Args &&... args) {
        link->template enqueue<Command>(std::forward<Args>(args)...);
    }

private:
    friend class Linker;
    CommandChannel<Group> *link = nullptr;
};

template<typename Group>
struct Dispatch<Group, Handle> {

    template<typename Command, auto Candidate, typename = typename std::enable_if_t<is_command_v<Command, Group>>>
    void connect() {
        link->template connect<Command, Candidate>();
    }

    template<typename Command, auto Candidate, typename Type,
            typename = typename std::enable_if_t<is_command_v<Command, Group>>>
    void connect(Type &&valueOrInstance) {
        link->template connect<Command, Candidate, Type>(std::forward<Type>(valueOrInstance));
    }

    void update() { link->update(); }

private:
    friend class Linker;
    CommandChannel<Group> *link = nullptr;
};

} // namespace rt
