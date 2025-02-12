#ifndef UTILS_NON_COPYABLE_HPP
#define UTILS_NON_COPYABLE_HPP

class NonCopyable {
    public:
        constexpr NonCopyable() = default;
        ~NonCopyable()          = default;
    
    protected:
        NonCopyable(NonCopyable&&)            = default;
        NonCopyable& operator=(NonCopyable&&) = default;
    
    private:
        NonCopyable(const NonCopyable&)      = delete;
        NonCopyable& operator=(NonCopyable&) = delete;
    };
    
#endif // UTILS_NON_COPYABLE_HPP
