#include "TaskScheduler.hpp"
#include "Application.hpp"

Task::~Task() {
    gerium_signal_destroy(_playSignal);
    gerium_signal_destroy(_cancelSignal);
}

Task::Task() {
    check(gerium_signal_create(&_cancelSignal));
    check(gerium_signal_create(&_playSignal));
}

void Task::callFn(Call fn, TaskState newState) noexcept {
    try {
        _state = newState;
        (this->*fn)(*_sheduler);
    } catch (...) {
        _error = std::current_exception();
        cancel();
    }
}

TaskScheduler::TaskScheduler(Application* applicaton) : _application(applicaton) {
    assert(applicaton);
    check(gerium_mutex_create(&_mutex));
    check(gerium_signal_create(&_close));
    check(gerium_signal_create(&_waitTasks));
}

TaskScheduler::~TaskScheduler() {
    close();
    gerium_signal_destroy(_waitTasks);
    gerium_signal_destroy(_close);
    gerium_mutex_destroy(_mutex);
}

void TaskScheduler::run() {
    if (_clonsing) {
        return;
    }
    auto running = false;
    if (!_running.compare_exchange_strong(running, true)) {
        return;
    }
    gerium_application_execute(_application->handle(), [](auto application, auto data) {
        auto scheduler = (TaskScheduler*) data;
        while (true) {
            gerium_signal_wait(scheduler->_waitTasks);
            if (!scheduler->_running) { // close() was called
                gerium_mutex_lock(scheduler->_mutex);
                deferred(gerium_mutex_unlock(scheduler->_mutex));
                if (scheduler->_tasks.empty()) { // So we start checking whether all tasks have been completed.
                    break;
                }
            }
            scheduler->process(); // Processing the task graph
        }
        gerium_signal_set(scheduler->_close); // We set a signal that control is complete
    }, this);
}

void TaskScheduler::close() {
    _clonsing = true;

    auto running = true;
    if (!_running.compare_exchange_strong(running, false)) {
        return;
    }
    while (!gerium_signal_is_set(_close)) { // Waiting for the end signal
        gerium_signal_set(_waitTasks);
        gerium_mutex_lock(_mutex);
        deferred(gerium_mutex_unlock(_mutex));
        for (auto task : _tasks) {
            // Request task cancellation. If tasks are spinning an infinite loop, they should react to this and return
            // control to from Task::process()
            task->cancel();
        }
    }
    gerium_signal_clear(_close);
    _clonsing = false;
}

void TaskScheduler::runTask(Task* task) {
    task->_state = TaskState::Stopped; // Until the fiber is started, the task will be in the stopped state.
    gerium_application_execute(_application->handle(), [](auto application, auto data) {
        auto task = (Task*) data;
        // Waiting for the start signal from the user
        gerium_signal_wait(task->_playSignal); 
        // 
        if (!task->isCancelled() && !task->hasError()) {
            task->callFn(&Task::process, TaskState::Executing);
        }
        task->callFn(&Task::onCancel, TaskState::Canceling);
    }, (gerium_data_t) task);
}

void TaskScheduler::waitTask(Task* task) {
    gerium_application_execute(_application->handle(), [](auto application, auto data) {
        auto task = (Task*) data;
        for (auto waitTask : task->_waitTasks) {
            gerium_signal_wait(waitTask->_cancelSignal);
        }
        task->_state = TaskState::Created;
    }, (gerium_data_t) task);
}

void TaskScheduler::addTask(Task* task) {
    {
        gerium_mutex_lock(_mutex);
        deferred(gerium_mutex_unlock(_mutex));
        _tasks.push_back(task);
    }
    gerium_signal_set(_waitTasks);
}

void TaskScheduler::process() {
    std::vector<Task*> tasks;
    {
        gerium_mutex_lock(_mutex);
        deferred(gerium_mutex_unlock(_mutex));
        tasks = _tasks;
    }
    for (auto task : tasks) {
        switch (task->state()) {
            case TaskState::Wait:
                if (task->isCancelled()) {
                    for (auto _waitTask : task->_waitTasks) {
                        _waitTask->onCancel(*this);
                    }
                }
                break;
            case TaskState::Created:
                task->callFn(&Task::onInitialize, TaskState::InQueue);
                break;
            case TaskState::InQueue:
                runTask(task);
                break;
            case TaskState::Canceling:
                if (task->_refs == 0) {
                    task->callFn(&Task::onUninitialize, TaskState::Ended);
                }
                break;
        }
    }
    removeEndedTasks();
}

void TaskScheduler::removeEndedTasks() {
    std::vector<Task*> tasks;
    {
        gerium_mutex_lock(_mutex);
        deferred(gerium_mutex_unlock(_mutex));
        std::erase_if(_tasks, [&tasks](auto task) {
            if (task->state() == TaskState::Ended) {
                tasks.push_back(task);
                return true;
            }
            return false;
        });
        if (_tasks.empty()) {
            gerium_signal_clear(_waitTasks);
        }
    }
    for (auto it = tasks.rbegin(); it != tasks.rend(); ++it) {
        delete *it;
    }
}
