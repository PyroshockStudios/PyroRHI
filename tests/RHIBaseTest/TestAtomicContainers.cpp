#include "gtest/gtest.h"
#include <PyroRHI/Common/AtomicMap.hpp>
#include <PyroRHI/Common/AtomicQueue.hpp>
#include <PyroRHI/Common/AtomicVector.hpp>
#include <EASTL/string.h>

using namespace PyroshockStudios::Common;

TEST(AtomicMap, EmplaceAndGet) {
    AtomicMap<int, int> map;
    map.Emplace(1, 100);
    map.Emplace(2, 200);
    EXPECT_EQ(map.Get(1), 100);
    EXPECT_EQ(map.Get(2), 200);
}

TEST(AtomicMap, GetOr) {
    AtomicMap<int, int> map;
    map.Emplace(1, 100);
    EXPECT_EQ(map.GetOr(1, -1), 100);
    EXPECT_EQ(map.GetOr(99, -1), -1);
}

TEST(AtomicMap, GetOpt) {
    AtomicMap<int, int> map;
    map.Emplace(1, 100);
    auto val = map.GetOpt(1);
    EXPECT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), 100);
    auto missing = map.GetOpt(99);
    EXPECT_FALSE(missing.has_value());
}

TEST(AtomicMap, Contains) {
    AtomicMap<int, int> map;
    map.Emplace(1, 100);
    EXPECT_TRUE(map.Contains(1));
    EXPECT_FALSE(map.Contains(2));
}

TEST(AtomicMap, Erase) {
    AtomicMap<int, int> map;
    map.Emplace(1, 100);
    EXPECT_TRUE(map.Contains(1));
    map.Erase(1);
    EXPECT_FALSE(map.Contains(1));
}

TEST(AtomicMap, EraseNonExistent) {
    AtomicMap<int, int> map;
    map.Erase(99);
    EXPECT_FALSE(map.Contains(99));
}

TEST(AtomicMap, Extract) {
    AtomicMap<int, int> map;
    map.Emplace(1, 100);
    int val = map.Extract(1);
    EXPECT_EQ(val, 100);
    EXPECT_FALSE(map.Contains(1));
}

TEST(AtomicMap, Size) {
    AtomicMap<int, int> map;
    EXPECT_EQ(map.Size(), 0u);
    map.Emplace(1, 100);
    EXPECT_EQ(map.Size(), 1u);
    map.Emplace(2, 200);
    EXPECT_EQ(map.Size(), 2u);
}

TEST(AtomicMap, Empty) {
    AtomicMap<int, int> map;
    EXPECT_TRUE(map.Empty());
    map.Emplace(1, 100);
    EXPECT_FALSE(map.Empty());
}

TEST(AtomicMap, Clear) {
    AtomicMap<int, int> map;
    map.Emplace(1, 100);
    map.Emplace(2, 200);
    map.Clear();
    EXPECT_TRUE(map.Empty());
    EXPECT_EQ(map.Size(), 0u);
}

TEST(AtomicMap, Set) {
    AtomicMap<int, int> map;
    map.Set(1, 100);
    EXPECT_EQ(map.Get(1), 100);
    map.Set(1, 999);
    EXPECT_EQ(map.Get(1), 100);
}

TEST(AtomicMap, ForEach) {
    AtomicMap<int, int> map;
    map.Emplace(1, 100);
    map.Emplace(2, 200);
    int sum = 0;
    map.ForEach([&sum](const int& key, int& val) { sum += val; });
    EXPECT_EQ(sum, 300);
}

TEST(AtomicMap, Ref) {
    AtomicMap<int, int> map;
    map.Emplace(1, 100);
    map.Ref(1, [](int& val) { val += 50; });
    EXPECT_EQ(map.Get(1), 150);
}

TEST(AtomicMap, RefOr) {
    AtomicMap<int, int> map;
    bool called = false;
    map.RefOr(99, [&called](auto opt) {
        called = true;
        EXPECT_FALSE(opt.has_value());
    });
    EXPECT_TRUE(called);
}

TEST(AtomicQueue, PushAndPop) {
    AtomicQueue<int> queue;
    queue.Push(10);
    queue.Push(20);
    EXPECT_EQ(queue.Size(), 2u);
    EXPECT_EQ(queue.Pop(), 10);
    EXPECT_EQ(queue.Pop(), 20);
    EXPECT_EQ(queue.Size(), 0u);
}

TEST(AtomicQueue, Empty) {
    AtomicQueue<int> queue;
    EXPECT_TRUE(queue.Empty());
    queue.Push(10);
    EXPECT_FALSE(queue.Empty());
}

TEST(AtomicQueue, Clear) {
    AtomicQueue<int> queue;
    queue.Push(10);
    queue.Push(20);
    queue.Clear();
    EXPECT_TRUE(queue.Empty());
    EXPECT_EQ(queue.Size(), 0u);
}

TEST(AtomicQueue, MovePush) {
    AtomicQueue<eastl::string> queue;
    eastl::string val = "hello";
    queue.Push(eastl::move(val));
    EXPECT_EQ(queue.Pop(), "hello");
}

TEST(AtomicVector, PushBackAndAt) {
    AtomicVector<int> vec;
    vec.PushBack(10);
    vec.PushBack(20);
    vec.PushBack(30);
    EXPECT_EQ(vec.At(0), 10);
    EXPECT_EQ(vec.At(1), 20);
    EXPECT_EQ(vec.At(2), 30);
}

TEST(AtomicVector, Size) {
    AtomicVector<int> vec;
    EXPECT_EQ(vec.Size(), 0u);
    vec.PushBack(10);
    EXPECT_EQ(vec.Size(), 1u);
}

TEST(AtomicVector, Empty) {
    AtomicVector<int> vec;
    EXPECT_TRUE(vec.Empty());
    vec.PushBack(10);
    EXPECT_FALSE(vec.Empty());
}

TEST(AtomicVector, Clear) {
    AtomicVector<int> vec;
    vec.PushBack(10);
    vec.PushBack(20);
    vec.Clear();
    EXPECT_TRUE(vec.Empty());
}

TEST(AtomicVector, Contains) {
    AtomicVector<int> vec;
    vec.PushBack(10);
    vec.PushBack(20);
    EXPECT_TRUE(vec.Contains(10));
    EXPECT_TRUE(vec.Contains(20));
    EXPECT_FALSE(vec.Contains(99));
}

TEST(AtomicVector, EraseByIndex) {
    AtomicVector<int> vec;
    vec.PushBack(10);
    vec.PushBack(20);
    vec.PushBack(30);
    vec.Erase(1);
    EXPECT_EQ(vec.Size(), 2u);
    EXPECT_EQ(vec.At(0), 10);
    EXPECT_EQ(vec.At(1), 30);
}

TEST(AtomicVector, EraseByRange) {
    AtomicVector<int> vec;
    vec.PushBack(10);
    vec.PushBack(20);
    vec.PushBack(30);
    vec.PushBack(40);
    vec.Erase(1, 3);
    EXPECT_EQ(vec.Size(), 2u);
    EXPECT_EQ(vec.At(0), 10);
    EXPECT_EQ(vec.At(1), 40);
}

TEST(AtomicVector, Assign) {
    AtomicVector<int> vec;
    vec.PushBack(10);
    vec.Assign(0, 99);
    EXPECT_EQ(vec.At(0), 99);
}

TEST(AtomicVector, PopBack) {
    AtomicVector<int> vec;
    vec.PushBack(10);
    vec.PushBack(20);
    int val = vec.PopBack();
    EXPECT_EQ(val, 20);
    EXPECT_EQ(vec.Size(), 1u);
}

TEST(AtomicVector, EmplaceBack) {
    AtomicVector<eastl::string> vec;
    vec.EmplaceBack("hello");
    EXPECT_EQ(vec.At(0), "hello");
}

TEST(AtomicVector, ForEach) {
    AtomicVector<int> vec;
    vec.PushBack(10);
    vec.PushBack(20);
    vec.PushBack(30);
    int sum = 0;
    vec.ForEach([&sum](int& val) { sum += val; });
    EXPECT_EQ(sum, 60);
}

TEST(AtomicVector, EraseFirstItem) {
    AtomicVector<int> vec;
    vec.PushBack(10);
    vec.PushBack(20);
    vec.PushBack(30);
    vec.EraseFirstItem(20);
    EXPECT_EQ(vec.Size(), 2u);
    EXPECT_FALSE(vec.Contains(20));
}
