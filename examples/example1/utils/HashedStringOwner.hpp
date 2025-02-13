#ifndef UTILS_HASHED_STRING_OWNER_HPP
#define UTILS_HASHED_STRING_OWNER_HPP

// Fast and reliable entity component system (ECS)
#include <entt/entt.hpp>

class string_owner {
protected:
    string_owner() = default;

    string_owner(const char* str, size_t len) : _str(str, len) {
    }

    string_owner(std::string&& str) noexcept : _str(std::move(str)) {
    }

    std::string _str{};
};

class hashed_string_owner : private string_owner,
                            public entt::hashed_string {
public:
    hashed_string_owner() noexcept = default;

    hashed_string_owner(const hashed_string_owner& owner) :
        string_owner(owner._str.c_str(), owner._str.length()),
        entt::hashed_string(_str.c_str(), _str.length()) {
    }

    hashed_string_owner(hashed_string_owner&& owner) noexcept :
        string_owner(std::move(owner._str)),
        entt::hashed_string(_str.c_str(), _str.length()) {
    }

    hashed_string_owner(const entt::hashed_string& str) :
        string_owner(str.data(), str.size()),
        entt::hashed_string(_str.c_str(), _str.length()) {
    }

    hashed_string_owner(const value_type* str, const size_type len) :
        string_owner(str, len),
        entt::hashed_string(_str.c_str(), _str.length()) {
    }

    template <size_t N>
    hashed_string_owner(const char (&str)[N]) :
        string_owner(str, N - 1),
        entt::hashed_string(_str.c_str(), _str.length()) {
    }

    hashed_string_owner(std::string&& str) noexcept :
        string_owner(std::move(str)),
        entt::hashed_string(_str.c_str(), _str.length()) {
    }

    hashed_string_owner(const char* str) :
        string_owner(str, strlen(str)),
        entt::hashed_string(_str.c_str(), _str.length()) {
    }

    [[nodiscard]] const std::string& string() const noexcept {
        return _str;
    }

    hashed_string_owner& operator=(const hashed_string_owner& owner) {
        if (this != &owner) {
            _str = owner._str;

            auto& hashedStr = *static_cast<entt::hashed_string*>(this);

            hashedStr = entt::hashed_string(_str.c_str(), _str.length());
        }
        return *this;
    }

    hashed_string_owner& operator=(hashed_string_owner&& owner) noexcept {
        if (this != &owner) {
            _str = std::move(owner._str);

            auto& hashedStr = *static_cast<entt::hashed_string*>(this);

            hashedStr = entt::hashed_string(_str.c_str(), _str.length());
        }
        return *this;
    }

    [[nodiscard]] bool operator==(const hashed_string_owner& rhs) const noexcept {
        return value() == rhs.value();
    }

    [[nodiscard]] bool operator!=(const hashed_string_owner& rhs) const noexcept {
        return !(*this == rhs);
    }

    [[nodiscard]] auto operator<=>(const hashed_string_owner& rhs) const noexcept {
        return value() <=> rhs.value();
    }
};

[[nodiscard]] inline hashed_string_owner operator""_hso(const char* str, std::size_t len) {
    return hashed_string_owner{ str, len };
}

#endif // UTILS_HASHED_STRING_OWNER_HPP
