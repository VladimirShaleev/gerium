#ifndef GERIUM_LOGGER_HPP
#define GERIUM_LOGGER_HPP

#include "ObjectPtr.hpp"

struct _gerium_logger : public gerium::Object {};

namespace gerium {

class Logger : public _gerium_logger {
public:
    explicit Logger(gerium_utf8_t tag);

    gerium_logger_level_t getLevel() const noexcept;
    void setLevel(gerium_logger_level_t level) noexcept;

    void print(gerium_logger_level_t level, gerium_utf8_t message) noexcept;

protected:
    virtual gerium_logger_level_t onGetLevel() const noexcept;
    virtual void onSetLevel(gerium_logger_level_t level) noexcept;

    virtual void onPrint(const std::string& tag, gerium_logger_level_t level, gerium_utf8_t message) = 0;

    static std::string_view levelToString(gerium_logger_level_t level) noexcept;

private:
    std::string _tag;
    gerium_logger_level_t _level;
};

} // namespace gerium

#endif
