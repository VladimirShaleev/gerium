#ifndef FINALLY_HPP
#define FINALLY_HPP

#include <utility>

template <typename F>
class Finally {
public:
    Finally(const Finally&) = delete;

    Finally& operator=(const Finally&) = delete;
    Finally& operator=(Finally&&)      = delete;

    Finally(const F& func) : _func(func) {
    }

    Finally(F&& func) : _func(std::move(func)) {
    }

    Finally(Finally&& other) : _func(std::move(other._func)) {
        other._valid = false;
    }

    ~Finally() {
        if (_valid) {
            _func();
        }
    }

private:
    F _func;
    bool _valid = true;
};

template <typename F>
inline Finally<F> makeFinally(F&& f) {
    return Finally<F>(std::forward<F>(f));
}

#define CONCAT_STRING_(a, b) a##b
#define CONCAT_STRING(a, b)  CONCAT_STRING_(a, b)
#define deferred(x)                                             \
    auto CONCAT_STRING(deferred_, __LINE__) = makeFinally([&] { \
        x;                                                      \
    })

#endif
