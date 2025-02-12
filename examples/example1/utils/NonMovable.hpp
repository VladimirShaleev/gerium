#ifndef UTILS_NON_MOVABLE_HPP
#define UTILS_NON_MOVABLE_HPP

#include "NonCopyable.hpp"

class NonMovable : NonCopyable {
    protected:
        constexpr NonMovable() = default;
        ~NonMovable()          = default;
    
    private:
        NonMovable(NonMovable&&)            = delete;
        NonMovable& operator=(NonMovable&&) = delete;
    };
    
#endif // UTILS_NON_MOVABLE_HPP
