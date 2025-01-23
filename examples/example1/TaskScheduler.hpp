#ifndef TASK_SCHEDULER_HPP
#define TASK_SCHEDULER_HPP

#include "Common.hpp"

class Application;
class TaskScheduler;

/// @brief Enumeration with task states
/// @details The state of a task will change atomically during the life of the task. A task starts its life in either
/// the Wait or Created state. The cycle is to reach the Ended state, after which the task is destroyed. States only
/// move upward, meaning that a task cannot return to an older state.
enum class TaskState {
    /// A task is in this state when it is waiting for other tasks to complete.
    Wait,

    /// This state indicates that the task has been created and is about to be queued for initialization.
    Created,

    /// This is the state in which the virtual method Task::initialize(TaskScheduler&) is called, which if it
    /// works successfully, the task can be started, otherwise the task will be cancelled.
    InQueue,

    /// Before starting or canceling the task, it is stopped.
    Stopped,

    /// The task is running, in this state the virtual method Task::process(TaskScheduler&) is called.
    Executing,

    /// The task is cancelling, in this state Task::uninitialize(TaskScheduler&) is called if initialization was called.
    /// *Please note that deinitialization, i.e. calling the Task::uninitialize(TaskScheduler&) method, will be done*
    /// *later than the initialization (Task::initialize(TaskScheduler&)) of all tasks that depend on it.*
    Canceling,

    /// The task has completed its work and will now be removed from memory.
    Ended
};

/// @brief The task from which custom tasks are inherited
/// @details In the task, you need to override at least the Task::process(TaskScheduler&) method.
class Task {
public:
    virtual ~Task(); ///< Virtual destructor

    Task(const Task&) = delete; ///< Copy constructor
    Task(Task&&)      = delete; ///< Move constructor

    Task& operator=(const Task&) = delete; ///< Copy operator
    Task& operator=(Task&&)      = delete; ///< Move operator

    /// @brief Get application ref
    /// @return Application ref
    Application& application() noexcept;

    /// @brief Get scheduler ref
    /// @return Scheduler ref
    TaskScheduler& sheduler() noexcept;

    /// @brief Get tasks that the current task depends on
    /// @return vector of tasks
    const std::vector<Task*>& dependencies() const noexcept;

    /// @brief Current state of task
    /// @return Current state
    TaskState state() const noexcept;

    /// @brief Check if the task is cancelled
    /// @return Success if canceled
    bool isCancelled() const noexcept;

    /// @brief Checking if there was an unhandled error in a task
    /// @return Success if there is an error
    bool hasError() const noexcept;

    /// @brief Cancel current task
    void cancel() noexcept;

    /// @brief Run task
    void run() noexcept;

protected:
    Task(); ///< Constructor

    /// @brief Called when the task is initialized
    /// @details If an uncaught exception is thrown in the task, the process will not be started. If the current task
    /// depends on other tasks, here you can get the list of tasks and get the result of their work, calling this method
    /// ensures that the dependent tasks have not been de-initialized yet.
    /// @param[in] sheduler TaskScheduler that created the task
    virtual void initialize(TaskScheduler& sheduler) {
    }

    /// @brief Called when the task is deinitialized
    /// @details You can safely clean up resources here
    /// @param[in] sheduler TaskScheduler that created the task
    virtual void uninitialize(TaskScheduler& sheduler) {
    }

    /// @brief Called when the task is deinitialized
    /// @param[in] sheduler TaskScheduler that created the task
    virtual void process(TaskScheduler& sheduler) = 0;

private:
    friend TaskScheduler;

    typedef void (Task::*Call)(TaskScheduler&);

    void callFn(Call fn, TaskState newState) noexcept;

    void onInitialize(TaskScheduler& sheduler) {
        // If the task has already been cancelled, there is no point in initializing it
        if (!isCancelled()) {
            _initialized = true;
            initialize(sheduler);
        }
        removeRefs();
    }

    void onUninitialize(TaskScheduler& sheduler) {
        // If the task has not been initialized, then there is no need to uninitialize it
        if (_initialized) {
            uninitialize(sheduler);
        }
    }

    void onCancel(TaskScheduler&) {
        // This is a signal that the task has been cancelled. This allows waiting dependent fibers of this fiber to be
        // woken up so that they can pick up the result of this fiber in the initialization method. After which, you can
        // safely deinitialize this task.
        gerium_signal_set(_cancelSignal);
    }

    void removeRefs() {
        // The counter is needed to prevent the destruction of tasks on which the current task depends. After the
        // current task is initialized, the counters of dependent tasks are decremented, and those tasks whose counter
        // reaches zero are uninitialized and destroyed.
        for (auto task : _waitTasks) {
            --task->_refs;
        }
        _waitTasks.clear();
    }

    gerium_signal_t _cancelSignal{}; // Signal that the task was cancelled
    gerium_signal_t _playSignal{};   // Signal that the task is ready to run
    TaskScheduler* _sheduler{};      // Pointer to scheduler
    std::atomic<TaskState> _state{}; // Current state
    std::exception_ptr _error{};     // Current error
    std::atomic_bool _initialized{}; // Was it initialized
    std::atomic_bool _cancel{};      // Was there a request to cancel the task
    std::atomic_size_t _refs{};      // Counter of references to the current task
    std::vector<Task*> _waitTasks{}; // List of tasks that the current task depends on
};

/// @brief Task for implementing lambdas (for internal use)
template <typename F>
class Func final : public Task {
public:
    Func(const Func&) = delete;
    Func(Func&&)      = delete;

    Func& operator=(const Func&) = delete;
    Func& operator=(Func&&)      = delete;

protected:
    void process(TaskScheduler& sheduler) override {
        _func(sheduler, (Task*) this);
    }

private:
    friend TaskScheduler;

    explicit Func(F&& func) noexcept : _func(std::move(func)) {
    }

    F _func;
};

/// @brief Scheduler for managing task graph
/// @details Its methods are thread-safe, adding tasks can be done from any fibers during their processing.
class TaskScheduler final {
public:
    /// @brief Constructor
    /// @param[in] applicaton Pointer of application
    explicit TaskScheduler(Application* applicaton);

    /// @brief Destructor
    ~TaskScheduler();

    TaskScheduler(const TaskScheduler&) = delete; ///< Copy constructor
    TaskScheduler(TaskScheduler&&)      = delete; ///< Move constructor

    TaskScheduler& operator=(const TaskScheduler&) = delete; ///< Copy operator
    TaskScheduler& operator=(TaskScheduler&&)      = delete; ///< Move operator

    /// @brief Get application ref
    /// @return Application ref
    Application& application() noexcept {
        return *_application;
    }

    /// @brief Add task
    /// @details The created task will not be executed. To run it, you need to call the Task::run() method
    /// @tparam T Type of task
    /// @tparam Args Arguments that will be passed to the constructor of the task being created
    /// @param[in] args Arguments for creating a task
    /// @return Created task
    template <typename T, typename... Args>
    T* addTask(Args&&... args) {
        return createTask<T>({}, std::forward<Args>(args)...);
    }

    /// @brief Add task
    /// @details Creates a task that will depend on other tasks. To make the task runnable, call Task::run(). The task
    /// will actually start when all dependent tasks have completed.
    /// @tparam T Type of task
    /// @tparam Args Arguments that will be passed to the constructor of the task being created
    /// @param[in] dependencies These are the tasks that we will be waiting for to be completed.
    /// @param[in] args Arguments for creating a task
    /// @return Created task
    template <typename T, typename... Args>
    T* thenTask(std::vector<Task*> dependencies, Args&&... args) {
        return createTask<T>(std::move(dependencies), std::forward<Args>(args)...);
    }

    /// @brief Add func
    /// @details The created task will not be executed. To run it, you need to call the Task::run() method
    /// @tparam F Type of lambda
    /// @param[in] func Lambda
    /// @return Created task
    template <typename F>
    Task* addFunc(F&& func) {
        return addTask<Func<F>>(std::forward<F>(func));
    }

    /// @brief Add func
    /// @details Creates a task that will depend on other tasks. To make the task runnable, call Task::run(). The task
    /// will actually start when all dependent tasks have completed.
    /// @tparam F Type of lambda
    /// @param[in] dependencies These are the tasks that we will be waiting for to be completed.
    /// @param[in] func Lambda
    /// @return Created task
    template <typename F>
    Task* thenFunc(std::vector<Task*> dependencies, F&& func) {
        return thenTask<Func<F>>(dependencies, std::forward<F>(func));
    }

    /// @brief Start scheduling
    /// @details Here a fiber is created, in which all the management of the task dependency graph will take place.
    void run();

    /// @brief Stop scheduling
    /// @details All tasks will be cancelled and the calling thread will be blocked to wait for all tasks to complete.
    void close();

private:
    template <typename T, typename... Args>
    T* createTask(std::vector<Task*>&& dependencies, Args&&... args) {
        static_assert(std::is_base_of_v<Task, T>, "T must be inherited from the base task Task");
        auto task = new T(std::forward<Args>(args)...);

        task->_sheduler = this;

        if (dependencies.empty()) {
            task->_state = TaskState::Created; // A task that has no dependencies is moved to the Created state
            addTask(task);                     // Add task to list for scheduler control
        } else {
            task->_state     = TaskState::Wait; // A task that has dependencies is moved to the Wait state
            task->_waitTasks = std::move(dependencies);
            for (auto task : task->_waitTasks) {
                ++task->_refs; // We increment the counters of the tasks we depend on
            }
            waitTask(task); // Creates a fiber that will wait for the tasks this task depends on to finish.
            addTask(task);  // Add task to list for scheduler control
        }
        return task;
    }

    void runTask(Task* task);
    void waitTask(Task* task);

    void addTask(Task* task);
    void process();
    void removeEndedTasks();

    Application* _application{};  // Pointer to application
    std::vector<Task*> _tasks{};  // Graph of tasks that are under management
    gerium_mutex_t _mutex{};      // Mutex for synchronization work with the task graph
    std::atomic_bool _running{};  // Was run() called
    std::atomic_bool _clonsing{}; // Was close() called
    gerium_signal_t _close{};     // Indicates that management and destruction of all tasks has been completed
    gerium_signal_t _waitTasks;   // Indicates that a task has appeared in the graph tasks
};

inline Application& Task::application() noexcept {
    return _sheduler->application();
}

inline TaskScheduler& Task::sheduler() noexcept {
    return *_sheduler;
}

inline const std::vector<Task*>& Task::dependencies() const noexcept {
    return _waitTasks;
}

inline TaskState Task::state() const noexcept {
    return _state;
}

inline bool Task::isCancelled() const noexcept {
    return _cancel;
}

inline bool Task::hasError() const noexcept {
    return !!_error;
}

inline void Task::cancel() noexcept {
    _cancel = true;
    run(); // If the task has not been started, we need to start it to begin processing it for cancellation
}

inline void Task::run() noexcept {
    // Set a signal that the task can be run. Tasks not run after creation until the run() method is called. This is
    // necessary in order to be able to add this task as a dependency for other tasks in some cases. Otherwise, if the
    // task were run immediately, it could have time to finish before we start creating a new task that will depend on
    // this one.
    gerium_signal_set(_playSignal);
}

#endif
