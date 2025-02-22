#ifndef UTILS_REFLECT_CPP_CONVERTERS_HPP
#define UTILS_REFLECT_CPP_CONVERTERS_HPP

#include "../Common.hpp"
#include "HashedStringOwner.hpp"

template <int N, typename T>
struct Vec {
    static constexpr glm::qualifier Q = glm::defaultp;

    std::array<T, N> v;

    static Vec from_class(const glm::vec<N, T, Q>& v) noexcept {
        Vec result;
        memcpy(&result, &v, sizeof(result));
        return result;
    }

    glm::vec<N, T, Q> to_class() const {
        glm::vec<N, T, Q> result;
        memcpy(&result, this, sizeof(result));
        return result;
    }
};

template <int R, int C, typename T>
struct Mat {
    static constexpr glm::qualifier Q = glm::defaultp;

    std::array<T, R * C> a;

    static Mat from_class(const glm::mat<R, C, T, Q>& m) noexcept {
        Mat result;
        memcpy(&result, &m, sizeof(result));
        return result;
    }

    glm::mat<R, C, T, Q> to_class() const {
        glm::mat<R, C, T, Q> result;
        memcpy(&result, this, sizeof(result));
        return result;
    }
};

template <typename T>
struct Quat {
    static constexpr glm::qualifier Q = glm::defaultp;

    T x, y, z, w;

    static Quat from_class(const glm::qua<T, Q>& q) noexcept {
        return { q.x, q.y, q.z, q.w };
    }

    glm::qua<T, Q> to_class() const {
        return { x, y, z, w };
    }
};

struct String {
    std::string str;
    gerium_uint32_t hash;

    static String from_class(const hashed_string_owner& str) noexcept {
        return { str.string(), str.value() };
    }

    hashed_string_owner to_class() const {
        return { str.c_str(), str.length() };
    }
};

struct Entity {
    gerium_uint32_t entity;

    static Entity from_class(const entt::entity& entity) noexcept {
        return { entity == entt::null ? std::numeric_limits<gerium_uint32_t>::max() : gerium_uint32_t(entity) };
    }

    entt::entity to_class() const {
        return entity == std::numeric_limits<gerium_uint32_t>::max() ? entt::null : entt::entity{ entity };
    }
};

namespace rfl {

namespace parsing {

template <class ReaderType, class WriterType, int N, class T, class ProcessorsType>
struct Parser<ReaderType, WriterType, glm::vec<N, T, glm::defaultp>, ProcessorsType>
    : public CustomParser<ReaderType, WriterType, ProcessorsType, glm::vec<N, T, glm::defaultp>, Vec<N, T>> {};

template <class ReaderType, class WriterType, int R, int C, class T, class ProcessorsType>
struct Parser<ReaderType, WriterType, glm::mat<R, C, T, glm::defaultp>, ProcessorsType>
    : public CustomParser<ReaderType, WriterType, ProcessorsType, glm::mat<R, C, T, glm::defaultp>, Mat<R, C, T>> {};

template <class ReaderType, class WriterType, class T, class ProcessorsType>
struct Parser<ReaderType, WriterType, glm::qua<T, glm::defaultp>, ProcessorsType>
    : public CustomParser<ReaderType, WriterType, ProcessorsType, glm::qua<T, glm::defaultp>, Quat<T>> {};

template <class ReaderType, class WriterType, class ProcessorsType>
struct Parser<ReaderType, WriterType, hashed_string_owner, ProcessorsType>
    : public CustomParser<ReaderType, WriterType, ProcessorsType, hashed_string_owner, String> {};

template <class ReaderType, class WriterType, class ProcessorsType>
struct Parser<ReaderType, WriterType, entt::entity, ProcessorsType>
    : public CustomParser<ReaderType, WriterType, ProcessorsType, entt::entity, Entity> {};

} // namespace parsing

} // namespace rfl

#endif // UTILS_REFLECT_CPP_CONVERTERS_HPP
