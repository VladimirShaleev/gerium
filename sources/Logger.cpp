#include "Logger.hpp"

using namespace std::string_literals;

namespace gerium {

Logger::Logger(gerium_utf8_t tag) :
    _tag(tag ? tag : ""s),
    _level(GERIUM_LOGGER_LEVEL_VERBOSE),
    _hashs(tagHashs(_tag)),
    _changed(true) {
    validateTag(tag);

    const std::lock_guard<std::mutex> lock(_mutex);
    _loggers.push_back(this);
}

Logger::~Logger() {
    const std::lock_guard<std::mutex> lock(_mutex);
    _loggers.erase(std::find(_loggers.begin(), _loggers.end(), this));
}

gerium_logger_level_t Logger::getLevelWithParent() noexcept {
    if (_changed) {
        const std::lock_guard<std::mutex> lock(_mutex);
        _changed = false;

        _level = GERIUM_LOGGER_LEVEL_VERBOSE;

        for (const gerium_uint64_t hash : _hashs) {
            auto it = _logLevels.find(hash);

            auto tagLevel = it != _logLevels.end() ? it->second : GERIUM_LOGGER_LEVEL_VERBOSE;
            if (_level < tagLevel) {
                _level = tagLevel;
                if (_level == GERIUM_LOGGER_LEVEL_OFF) {
                    break;
                }
            }
        }
    }
    return _level;
}

gerium_logger_level_t Logger::getLevel() const noexcept {
    const std::lock_guard<std::mutex> lock(_mutex);
    return _logLevels[_hashs.front()];
}

void Logger::setLevel(gerium_logger_level_t level) noexcept {
    setLevel(_hashs.front(), level);
}

void Logger::setLevel(gerium_utf8_t tag, gerium_logger_level_t level) noexcept {
    std::string t = tag ? tag : ""s;
    validateTag(t);
    setLevel(hash(t), level);
}

void Logger::print(gerium_logger_level_t level, gerium_utf8_t message) noexcept {
    if (level == GERIUM_LOGGER_LEVEL_OFF) {
        return;
    }

    if (level >= getLevelWithParent()) {
        invoke<Logger>([this, level, message](auto obj) {
            obj->onPrint(_tag, level, message);
        });
    }
}

std::string_view Logger::levelToString(gerium_logger_level_t level) noexcept {
    assert(level >= GERIUM_LOGGER_LEVEL_VERBOSE && level <= GERIUM_LOGGER_LEVEL_OFF);
    constexpr std::string_view levels[] = { "VERBOSE", "DEBUG", "INFO", "WARNING", "ERROR", "FATAL", "OFF" };
    return levels[(int) level];
}

void Logger::validateTag(const std::string& tag) {
    if (ctre::search<":\\s*:">(tag)) {
        error(GERIUM_RESULT_ERROR_INVALID_ARGUMENT);
    }
}

void Logger::setLevel(gerium_uint64_t hash, gerium_logger_level_t level) noexcept {
    const std::lock_guard<std::mutex> lock(_mutex);
    if (_logLevels[hash] != level) {
        _logLevels[hash] = level;
        for (auto& logger : _loggers) {
            logger->_changed = true;
        }
    }
}

std::vector<gerium_uint64_t> Logger::tagHashs(const std::string& tag) {
    std::vector<gerium_uint64_t> result;
    result.reserve(4);

    std::string currentTag = tag;

    gerium_logger_level_t currentLevel = GERIUM_LOGGER_LEVEL_VERBOSE;

    while (currentTag.length()) {
        result.push_back(hash(currentTag));
        if (auto pos = currentTag.rfind(':'); pos == std::string::npos) {
            currentTag = ""s;
        } else {
            currentTag = currentTag.substr(0, pos);
        }
    }

    result.push_back(hash(""s));
    return result;
}

std::vector<Logger*> Logger::_loggers = {};

std::map<gerium_uint64_t, gerium_logger_level_t> Logger::_logLevels = {};

std::mutex Logger::_mutex = {};

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

void gerium_logger_set_level_by_tag(gerium_utf8_t tag, gerium_logger_level_t level) {
    return Logger::setLevel(tag, level);
}

void gerium_logger_print(gerium_logger_t logger, gerium_logger_level_t level, gerium_utf8_t message) {
    assert(logger);
    assert(level >= GERIUM_LOGGER_LEVEL_VERBOSE && level <= GERIUM_LOGGER_LEVEL_OFF);
    assert(message);
    return alias_cast<Logger*>(logger)->print(level, message);
}
