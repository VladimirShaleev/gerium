#ifndef EVENT_HPP
#define EVENT_HPP

#include "../Common.hpp"

class Event {
public:
    virtual ~Event() = default;

    virtual int type() const noexcept = 0;

private:
    friend class EventManager;
    std::unique_ptr<Event> _next{};
};

#endif
