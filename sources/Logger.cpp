#include "Logger.hpp"

namespace gerium {

Logger::Logger(gerium_utf8_t tag) : _tag(tag), _level(GERIUM_LOGGER_LEVEL_VERBOSE) {
}

gerium_logger_level_t Logger::getLevel() const noexcept {
    return onGetLevel();
}

void Logger::setLevel(gerium_logger_level_t level) noexcept {
    onSetLevel(level);
}

void Logger::print(gerium_logger_level_t level, gerium_utf8_t message) noexcept {
    if (level >= _level) {
        invoke<Logger>([this, level, message](auto obj) {
            obj->onPrint(_tag, level, message);
        });
    }
}

gerium_logger_level_t Logger::onGetLevel() const noexcept {
    return _level;
}

void Logger::onSetLevel(gerium_logger_level_t level) noexcept {
    _level = level;
}

std::string_view Logger::levelToString(gerium_logger_level_t level) noexcept {
    assert(level >= GERIUM_LOGGER_LEVEL_VERBOSE && level <= GERIUM_LOGGER_LEVEL_FATAL);
    constexpr std::string_view levels[] = { "VERBOSE", "DEBUG", "INFO", "WARNING", "ERROR", "FATAL" };
    return levels[(int) level];
}

} // namespace gerium

using namespace gerium;

gerium_logger_t gerium_logger_reference(gerium_logger_t logger) {
    assert(logger);
    logger->reference();
    return logger;
}

void gerium_logger_destroy(gerium_logger_t logger) {
    if (logger) {
        logger->destroy();
    }
}

gerium_logger_level_t gerium_logger_get_level(gerium_logger_t logger) {
    assert(logger);
    return alias_cast<Logger*>(logger)->getLevel();
}

void gerium_logger_set_level(gerium_logger_t logger, gerium_logger_level_t level) {
    assert(logger);
    return alias_cast<Logger*>(logger)->setLevel(level);
}

void gerium_logger_print(gerium_logger_t logger, gerium_logger_level_t level, gerium_utf8_t message) {
    assert(logger);
    assert(level >= GERIUM_LOGGER_LEVEL_VERBOSE && level <= GERIUM_LOGGER_LEVEL_FATAL);
    assert(message);
    return alias_cast<Logger*>(logger)->print(level, message);
}
