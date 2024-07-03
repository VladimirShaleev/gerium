#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <ResourcePool.hpp>

TEST(ResourcePool, ObtainRelease) {
    struct Simple {
        int i1;
        int i2;
    };

    struct SimpleHandle : gerium::Handle {};

    using Pool = gerium::ResourcePool<Simple, SimpleHandle>;

    Pool pool;
}
