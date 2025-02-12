#ifndef UTILS_REFLECT_CPP_CONVERTERS_HPP
#define UTILS_REFLECT_CPP_CONVERTERS_HPP

#include "../Common.hpp"

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

} // namespace parsing

} // namespace rfl

#endif // UTILS_REFLECT_CPP_CONVERTERS_HPP
