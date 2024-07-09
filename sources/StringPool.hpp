#ifndef GERIUM_STRING_POOL_HPP
#define GERIUM_STRING_POOL_HPP

#include "Gerium.hpp"

namespace gerium {

class StringPool final {
public:
    explicit StringPool(size_t size = 1024) : _bucketSize(size), _offset(0) {
        addBucket();
    }

    StringPool(const StringPool&)            = delete;
    StringPool& operator=(const StringPool&) = delete;

    StringPool(StringPool&& other) noexcept {
        std::swap(_bucketSize, other._bucketSize);
        std::swap(_offset, other._offset);
        _buckets.swap(other._buckets);
        _table.swap(other._table);
    }

    StringPool& operator=(StringPool&& other) noexcept {
        if (this != &other) {
            clear();
            std::swap(_bucketSize, other._bucketSize);
            std::swap(_offset, other._offset);
            _buckets.swap(other._buckets);
            _table.swap(other._table);
        }
        return *this;
    }

    const char* intern(const char* str) {
        return str ? intern(std::string_view{ str }) : nullptr;
    }

    gerium_utf8_t intern(std::string_view str) {
        if (str.length() == 0) {
            return nullptr;
        }

        auto key = hash(str);
        if (auto it = _table.find(key); it != _table.end()) {
            return it->second;
        }

        auto len = str.length() + 1;
        if (_offset + len >= _bucketSize) {
            addBucket();
        }

        auto ptr = currentBucket() + _offset;
        memcpy((void*) ptr, str.data(), len);
        _offset += len;

        _table[key] = ptr;
        return ptr;
    }

    void clear() noexcept {
        _buckets.clear();
        _table.clear();
        addBucket();
    }

private:
    using bucket_type  = std::vector<char>;
    using buckets_type = std::list<bucket_type>;
    using map_type     = absl::flat_hash_map<gerium_uint64_t, gerium_utf8_t>;

    void addBucket() {
        _buckets.push_back({});
        _buckets.back().resize(_bucketSize);
        _offset = 0;
    }

    gerium_utf8_t currentBucket() {
        return _buckets.back().data();
    }

    size_t _bucketSize;
    size_t _offset;
    buckets_type _buckets;
    map_type _table;
};

gerium_utf8_t intern(const char* str);

gerium_utf8_t intern(std::string_view str);

} // namespace gerium

#endif
