#include "EventManager.hpp"

void EventManager::dispatch() {
    while (_queue) {
        send(*_queue.get());
        _queue = std::move(_queue->_next);
    }
    _last = nullptr;
}

void EventManager::send(const Event& event) {
    auto [itBegin, itEnd] = _subscribers.equal_range(event.type());

    for (auto it = itBegin; it != itEnd; ++it) {
        auto& subscriber = it->second;
        subscriber->call(event);
    }
}
