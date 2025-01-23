#ifndef EVENT_MANAGER_HPP
#define EVENT_MANAGER_HPP

#include "Event.hpp"

template <typename E>
using EventHandler = std::function<void(const E&)>;

class Subscriber : NonMovable {
public:
    virtual ~Subscriber() = default;
};

class EventManager final {
public:
    template <typename E>
    static int getEventType() noexcept {
        return typeId<E>;
    }

    template <typename E, typename... Args>
    void trigger(Args&&... args) {
        EventImpl<E, Args...> event(std::forward<Args>(args)...);
        send(event);
    }

    template <typename E, typename... Args>
    void send(Args&&... args) {
        auto event = std::make_unique<EventImpl<E, Args...>>(std::forward<Args>(args)...);
        if (_last) {
            _last->_next = std::move(event);
            _last        = _last->_next.get();
        } else {
            _queue = std::move(event);
            _last  = _queue.get();
        }
    }

    template <typename E>
    Subscriber* subscribe(EventHandler<E>&& handler) {
        struct TypeSubscriber : SubscriberCall {
            TypeSubscriber(EventHandler<E>&& handler) noexcept : _handler(std::move(handler)) {
            }

            void call(const Event& event) override {
                if (event.type() == getEventType<E>()) {
                    _handler((const E&) event);
                }
            }

            EventHandler<E> _handler{};
        };

        auto subscriber = std::make_unique<TypeSubscriber>(std::move(handler));
        auto result     = subscriber.get();

        _subscribers.insert({ getEventType<E>(), std::move(subscriber) });

        return result;
    }

    bool unsubscribe(Subscriber* subscriber) {
        auto it = std::find_if(_subscribers.begin(), _subscribers.end(), [subscriber](const auto& item) {
            return item.second.get() == subscriber;
        });
        if (it != _subscribers.end()) {
            _subscribers.erase(it);
            return true;
        }
        return false;
    }

    void dispatch();

private:
    class SubscriberCall : public Subscriber {
    public:
        virtual void call(const Event& event) = 0;
    };

    template <typename E, typename... Args>
    class EventImpl : public E {
    public:
        EventImpl(Args&&... args) : E(std::forward<Args>(args)...) {
        }

        int type() const noexcept override {
            return getEventType<E>();
        }
    };

    void send(const Event& event);

    std::multimap<int, std::unique_ptr<SubscriberCall>> _subscribers{};
    std::unique_ptr<Event> _queue{};
    Event* _last{};
};

#endif
