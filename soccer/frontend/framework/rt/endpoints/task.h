#pragma once

#include "endpoint.h"
#include "input.h"
#include "output.h"

#include <atomic>

namespace rt {

class Linker;

static constexpr Flag Dispatch      = OutputFlag;
static constexpr Flag TaskHandle    = OptionalInput;

namespace  detail {
    static constexpr Flag TaskFlags = Dispatch | TaskHandle;
    static constexpr Flag TaskInputFlags = Event;
    static constexpr Flag TaskOutputFlags = Event;
    
    static constexpr Flag getTaskFlags(Flag flags) {
        /* uncomment to make logging default
        if (not flag_set(flags, rt::DisableLogging)) {
            return flags | EnableLogging;
        }
        */
        return flags;
    }
    
    template<typename T, Flag Flags>
    class Task {
        static_assert(std::is_same_v<decltype(Flags), std::false_type>, "unsupported flag!");
    };

    template<typename ContextT>
    struct TaskMetadata {
        ContextT context;

        TaskMetadata(ContextT context)
        : context(std::move(context)) {

        }
    };
    
    template<typename T>
    class TaskData {
    public:
        using context_type = T;
        using result_type = typename T::result_type;
        using result_channel_type = Output<result_type, Event>;

        TaskData() = default;
        
        TaskData(std::shared_ptr<TaskMetadata<context_type>> meta)
            : meta(meta) {
        }

        bool unique() {
            return meta.unique();
        }

        const context_type& getContext() const {
            return meta->context;
        }

    private:
        std::shared_ptr<TaskMetadata<context_type>> meta {nullptr};
    };
    
    template<typename ContextT>
    class TaskResultEmitter {
    public:
        using result_type = typename ContextT::result_type;
        using task_type = TaskData<ContextT>;
        using result_message = std::tuple<task_type, result_type>;

        void emitResult(task_type &task, result_type &data) {
            channel.emit(std::make_tuple(task, data));
        }

        void emitResult(task_type &task, result_type &&data) {
            channel.emit(std::make_tuple(task, data));
        }

    protected:
        friend class rt::Linker;
        
        rt::Output<result_message, Event> channel;

        void linkResultEmitter(rt::Linker &link) {
            link(channel);
        }
    };
 
    template<typename ContextT>
    class Task<ContextT, TaskHandle> : public TaskResultEmitter<ContextT> {
    public:
        using task_type = TaskData<ContextT>;

        std::vector<task_type> fetch() { return channel.fetch(); }
        void waitWhileEmpty() { channel.waitWhileEmpty(); }

    private:
        friend class rt::Linker;
        rt::Input<task_type, Event> channel;
        
        void link(rt::Linker &link) {
            link(channel);
            this->linkResultEmitter(link);
        }
    };

    template<typename ContextT>
    class Task<ContextT, Dispatch> {
    public:
        using context_type = ContextT;
        using task_type = TaskData<ContextT>;
        using result_type = typename ContextT::result_type;
        using result_message = std::tuple<task_type, result_type>;

        void emit(context_type &data) {
            auto meta = std::make_shared<TaskMetadata<context_type>>(data);
            tasks.emplace_back(meta);
            dispatcher.emit(task_type(meta));
        }

        void emit(context_type &&data) {
            auto meta = std::make_shared<TaskMetadata<context_type>>(data);
            tasks.emplace_back(meta);
            dispatcher.emit(task_type(meta));
        }

        std::vector<result_message> fetch() { return result.fetch(); }
        void waitWhileEmpty() { result.waitWhileEmpty(); }

        std::vector<task_type> fetchComplete() {
            std::vector<task_type> result;
            auto end = std::remove_if(std::begin(tasks), std::end(tasks), [&result](task_type &task) {
                bool pred = task.unique();
                if(pred) {
                    result.push_back(task);
                }
                return pred;
            });
            tasks.erase(end, std::end(tasks));
            return result;
        }

    private:
        friend class rt::Linker;

        std::vector<task_type> tasks;

        rt::Output<task_type, Event> dispatcher;
        rt::Input<result_message, Event> result;
        
        void link(rt::Linker &link) {
            link(dispatcher);
            link(result);
        }
    };

    template<typename ContextT, typename T, Flag Flags>
    class TaskOutput : public TaskResultEmitter<ContextT> {
    public:
        using task_type = TaskData<ContextT>;
        using message_type = std::tuple<task_type, T>;
        using inner_type = rt::Output<message_type, Flags>;
        static constexpr Flag flags = inner_type::flags & ~EnableLogging;

        void emit(task_type &task, const T& data) {
           output.emit(std::make_tuple(task, data)); 
        }

    private:
        friend class rt::Linker;
        inner_type output;
        
        void link(rt::Linker &link) {
            link(output);
            this->linkResultEmitter(link);
        }
    };

    template<typename ContextT, typename T, Flag Flags>
    class TaskInput : public TaskResultEmitter<ContextT> {
    public:
        using task_type = TaskData<ContextT>;
        using message_type = std::tuple<task_type, T>;
        using inner_type = rt::Input<message_type, Flags>;
        static constexpr Flag flags = inner_type::flags & ~EnableLogging;

        std::vector<message_type> fetch() { return input.fetch(); }
        void waitWhileEmpty() { input.waitWhileEmpty(); }

    private:
        friend class rt::Linker;
        inner_type input;
        
        void link(rt::Linker &link) {
            link(input);
            this->linkResultEmitter(link);
        }
    };

}

template<typename ContextT, Flag Flags = TaskHandle>
struct Task : public detail::Task<ContextT, Flags> {
    static constexpr Flag flags = detail::getTaskFlags(Flags);
};

template<typename ContextT, typename T, Flag Flags = Event>
struct TaskInput : public detail::TaskInput<ContextT, T, Flags> {
    using detail::TaskInput<ContextT, T, Flags>::flags;
};

template<typename ContextT, typename T, Flag Flags = Event>
struct TaskOutput : public detail::TaskOutput<ContextT, T, Flags> {
    using detail::TaskOutput<ContextT, T, Flags>::flags;
};

}
