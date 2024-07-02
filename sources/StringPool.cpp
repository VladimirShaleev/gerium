#include "StringPool.hpp"

namespace gerium {

static StringPool pool;

gerium_utf8_t intern(const char* str) {
    return pool.intern(str);
}

gerium_utf8_t intern(std::string_view str) {
    return pool.intern(str);
}

} // namespace gerium
