#ifndef GERIUM_LOGGER_HPP
#define GERIUM_LOGGER_HPP

#include "ObjectPtr.hpp"

struct _gerium_logger : public gerium::Object {};

namespace gerium {

class Logger : public _gerium_logger {
public:
    explicit Logger(gerium_utf8_t tag);
    ~Logger();

    gerium_logger_level_t getLevelWithParent() noexcept;

    gerium_logger_level_t getLevel() const noexcept;
    void setLevel(gerium_logger_level_t level) noexcept;

    static void setLevel(gerium_utf8_t tag, gerium_logger_level_t level) noexcept;

    void print(gerium_logger_level_t level, gerium_utf8_t message) noexcept;

protected:
    virtual void onPrint(const std::string& tag, gerium_logger_level_t level, gerium_utf8_t message) = 0;

    static std::string_view levelToString(gerium_logger_level_t level) noexcept;

private:
    static void validateTag(const std::string& tag);
    static void setLevel(gerium_uint64_t hash, gerium_logger_level_t level) noexcept;
    static std::vector<gerium_uint64_t> tagHashs(const std::string& tag);

    std::string _tag;
    gerium_logger_level_t _level;
    std::vector<gerium_uint64_t> _hashs;
    std::atomic_bool _changed;
    
    static std::vector<Logger*> _loggers;
    static std::map<gerium_uint64_t, gerium_logger_level_t> _logLevels;
    static std::mutex _mutex;
};

} // namespace gerium

#endif
