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
    ASSERT_TRUE(pool.empty());
    ASSERT_EQ(pool.size(), 0);

    auto [h1, s1] = pool.obtain_and_access();
    s1->i1        = 10;
    s1->i2        = 100;
    ASSERT_EQ(h1.index, 0);
    ASSERT_EQ(pool.handle(s1).index, 0);
    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 1);
    ASSERT_EQ(s1->i1, 10);
    ASSERT_EQ(s1->i2, 100);

    auto [h2, s2] = pool.obtain_and_access();
    s2->i1        = 20;
    s2->i2        = 200;
    ASSERT_EQ(h2.index, 1);
    ASSERT_EQ(pool.handle(s2).index, 1);
    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 2);
    ASSERT_EQ(s2->i1, 20);
    ASSERT_EQ(s2->i2, 200);

    auto [h3, s3] = pool.obtain_and_access();
    s3->i1        = 30;
    s3->i2        = 300;
    ASSERT_EQ(h3.index, 2);
    ASSERT_EQ(pool.handle(s3).index, 2);
    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 3);
    ASSERT_EQ(s3->i1, 30);
    ASSERT_EQ(s3->i2, 300);

    auto [h4, s4] = pool.obtain_and_access();
    s4->i1        = 40;
    s4->i2        = 400;
    ASSERT_EQ(h4.index, 3);
    ASSERT_EQ(pool.handle(s4).index, 3);
    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 4);
    ASSERT_EQ(s4->i1, 40);
    ASSERT_EQ(s4->i2, 400);

    auto [h5, s5] = pool.obtain_and_access();
    s5->i1        = 50;
    s5->i2        = 500;
    ASSERT_EQ(h5.index, 4);
    ASSERT_EQ(pool.handle(s5).index, 4);
    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 5);
    ASSERT_EQ(s5->i1, 50);
    ASSERT_EQ(s5->i2, 500);

    auto [h6, s6] = pool.obtain_and_access();
    s6->i1        = 60;
    s6->i2        = 600;
    ASSERT_EQ(h6.index, 5);
    ASSERT_EQ(pool.handle(s6).index, 5);
    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 6);
    ASSERT_EQ(s6->i1, 60);
    ASSERT_EQ(s6->i2, 600);

    pool.release(h1);
    ASSERT_EQ(pool.size(), 5);

    pool.release(h2);
    ASSERT_EQ(pool.size(), 4);

    pool.release(h3);
    ASSERT_EQ(pool.size(), 3);

    auto [h7, s7] = pool.obtain_and_access();
    s7->i1        = 70;
    s7->i2        = 700;
    ASSERT_EQ(h7.index, 2);
    ASSERT_EQ(pool.handle(s7).index, 2);
    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 4);
    ASSERT_EQ(s7->i1, 70);
    ASSERT_EQ(s7->i2, 700);

    auto [h8, s8] = pool.obtain_and_access();
    s8->i1        = 80;
    s8->i2        = 800;
    ASSERT_EQ(h8.index, 1);
    ASSERT_EQ(pool.handle(s8).index, 1);
    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 5);
    ASSERT_EQ(s8->i1, 80);
    ASSERT_EQ(s8->i2, 800);

    auto [h9, s9] = pool.obtain_and_access();
    s9->i1        = 90;
    s9->i2        = 900;
    ASSERT_EQ(h9.index, 0);
    ASSERT_EQ(pool.handle(s9).index, 0);
    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 6);
    ASSERT_EQ(s9->i1, 90);
    ASSERT_EQ(s9->i2, 900);

    pool.release(h4);
    ASSERT_EQ(pool.size(), 5);

    pool.release(h5);
    ASSERT_EQ(pool.size(), 4);

    pool.release(h6);
    ASSERT_EQ(pool.size(), 3);

    auto [h10, s10] = pool.obtain_and_access();
    s10->i1         = 100;
    s10->i2         = 1000;
    ASSERT_EQ(h10.index, 5);
    ASSERT_EQ(pool.handle(s10).index, 5);
    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 4);
    ASSERT_EQ(s10->i1, 100);
    ASSERT_EQ(s10->i2, 1000);

    auto [h11, s11] = pool.obtain_and_access();
    s11->i1         = 110;
    s11->i2         = 1100;
    ASSERT_EQ(h11.index, 4);
    ASSERT_EQ(pool.handle(s11).index, 4);
    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 5);
    ASSERT_EQ(s11->i1, 110);
    ASSERT_EQ(s11->i2, 1100);

    auto [h12, s12] = pool.obtain_and_access();
    s12->i1         = 120;
    s12->i2         = 1200;
    ASSERT_EQ(h12.index, 3);
    ASSERT_EQ(pool.handle(s12).index, 3);
    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 6);
    ASSERT_EQ(s12->i1, 120);
    ASSERT_EQ(s12->i2, 1200);
}

TEST(ResourcePool, Resize) {
    struct Simple {
        int i1;
        int i2;
    };

    struct SimpleHandle : gerium::Handle {};

    using Pool = gerium::ResourcePool<Simple, SimpleHandle>;

    Pool pool(3);
    ASSERT_TRUE(pool.empty());
    ASSERT_EQ(pool.size(), 0);
    ASSERT_EQ(pool.capacity(), 3);

    pool.obtain();
    pool.obtain();
    pool.obtain();
    ASSERT_EQ(pool.size(), 3);
    ASSERT_EQ(pool.capacity(), 3);

    pool.obtain();
    pool.obtain();
    ASSERT_EQ(pool.size(), 5);
    ASSERT_EQ(pool.capacity(), 6);
}

TEST(ResourcePool, References) {
    struct Simple {
        int i1;
        int i2;
    };

    struct SimpleHandle : gerium::Handle {};

    using Pool = gerium::ResourcePool<Simple, SimpleHandle>;

    Pool pool;

    auto [handle, simple] = pool.obtain_and_access();
    ASSERT_EQ(pool.size(), 1);

    pool.addReference(handle);
    ASSERT_EQ(pool.size(), 1);

    ASSERT_FALSE(pool.release(handle));
    ASSERT_EQ(pool.size(), 1);

    ASSERT_TRUE(pool.release(handle));
    ASSERT_EQ(pool.size(), 0);
}

TEST(ResourcePool, ForEach) {
    struct Simple {
        int i;
    };

    struct SimpleHandle : gerium::Handle {};

    using Pool = gerium::ResourcePool<Simple, SimpleHandle>;

    Pool pool;

    for (const auto& s : pool) {
    }
    auto diff0 = pool.end() - pool.begin();
    ASSERT_EQ(diff0, 0);

    auto [h1, s1] = pool.obtain_and_access();
    s1->i         = 1;

    for (const auto& s : pool) {
        ASSERT_EQ(s->i, 1);
    }
    auto diff1 = pool.end() - pool.begin();
    ASSERT_EQ(diff1, 1);

    auto [h2, s2] = pool.obtain_and_access();
    auto [h3, s3] = pool.obtain_and_access();
    auto [h4, s4] = pool.obtain_and_access();
    s2->i         = 2;
    s3->i         = 3;
    s4->i         = 4;

    pool.release(h3);
    pool.release(h2);

    auto [h5, s5] = pool.obtain_and_access();
    auto [h6, s6] = pool.obtain_and_access();
    auto [h7, s7] = pool.obtain_and_access();
    s5->i         = 5;
    s6->i         = 6;
    s7->i         = 7;

    pool.release(h4);
    pool.release(h5);
    pool.release(h1);

    std::queue<int> vals;
    vals.push(6);
    vals.push(7);
    for (const auto& s : pool) {
        ASSERT_EQ(s->i, vals.front());
        vals.pop();
    }

    auto diff2 = pool.end() - pool.begin();
    ASSERT_EQ(diff2, 2);
}
