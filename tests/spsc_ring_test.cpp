#include <gtest/gtest.h>

#include <cstddef>
#include <edgesense/concurrency/spsc_ring.hpp>
#include <memory>
#include <string>
#include <thread>

namespace {

using edgesense::SpscRing;

TEST(SpscRing, StartsEmpty) {
  SpscRing<int, 4> q;
  EXPECT_FALSE(q.try_pop().has_value());
}

TEST(SpscRing, PopReturnsPushedValue) {
  SpscRing<int, 4> q;
  ASSERT_TRUE(q.try_push(42));
  auto v = q.try_pop();
  ASSERT_TRUE(v.has_value());
  EXPECT_EQ(*v, 42);
  EXPECT_FALSE(q.try_pop().has_value());  // empty again
}

// One slot is always kept free to tell "full" from "empty", so N=4 holds 3 items.
TEST(SpscRing, FullAtCapacityMinusOne) {
  SpscRing<int, 4> q;
  EXPECT_TRUE(q.try_push(1));
  EXPECT_TRUE(q.try_push(2));
  EXPECT_TRUE(q.try_push(3));
  EXPECT_FALSE(q.try_push(4));  // full

  ASSERT_EQ(q.try_pop(), 1);
  EXPECT_TRUE(q.try_push(4));  // one slot freed
  EXPECT_FALSE(q.try_push(5));
}

TEST(SpscRing, FifoOrderWhenFilledAndDrained) {
  SpscRing<int, 8> q;
  for (int i = 0; i < 7; ++i) ASSERT_TRUE(q.try_push(i));
  for (int i = 0; i < 7; ++i) EXPECT_EQ(q.try_pop(), i);
  EXPECT_FALSE(q.try_pop().has_value());
}

// Push 2 / pop 1 and then drain, over many rounds, so head and tail cross the end of
// the buffer at different offsets.
TEST(SpscRing, WrapsAroundKeepingOrder) {
  SpscRing<int, 4> q;
  int next_in = 0;
  int next_out = 0;
  for (int round = 0; round < 50; ++round) {
    ASSERT_TRUE(q.try_push(next_in++));
    ASSERT_TRUE(q.try_push(next_in++));
    ASSERT_EQ(q.try_pop(), next_out++);
    ASSERT_EQ(q.try_pop(), next_out++);
  }
  // Fill completely after many wraps: capacity must still be N - 1.
  for (int i = 0; i < 3; ++i) ASSERT_TRUE(q.try_push(i));
  EXPECT_FALSE(q.try_push(99));
}

// A heap-owning type: under ASan this catches use-after-free or double-free in
// copy/assignment paths that an int would never exercise.
TEST(SpscRing, WorksWithNonTrivialType) {
  SpscRing<std::string, 4> q;
  const std::string longer_than_sso(100, 'x');
  for (int round = 0; round < 10; ++round) {
    ASSERT_TRUE(q.try_push(longer_than_sso + std::to_string(round)));
    auto v = q.try_pop();
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, longer_than_sso + std::to_string(round));
  }
}

TEST(SpscRing, WorksWithMoveOnlyType) {
  SpscRing<std::unique_ptr<int>, 4> q;
  for (int round = 0; round < 10; ++round) {  // several wraps of a 3-slot ring
    ASSERT_TRUE(q.try_push(std::make_unique<int>(round)));
    auto v = q.try_pop();
    ASSERT_TRUE(v.has_value());
    ASSERT_NE(*v, nullptr);
    EXPECT_EQ(**v, round);
  }
}

TEST(SpscRing, FailedPushLeavesValueIntact) {
  SpscRing<std::unique_ptr<int>, 2> q;  // capacity 1
  ASSERT_TRUE(q.try_push(std::make_unique<int>(1)));
  auto p = std::make_unique<int>(2);
  EXPECT_FALSE(q.try_push(std::move(p)));
  ASSERT_NE(p, nullptr);  // a rejected push must not steal the value
  EXPECT_EQ(*p, 2);
}

TEST(SpscRing, ProduceAndConsume) {
  SpscRing<long, 64> q;
  constexpr long M = 1'000'000;
  long sum = 0, received = 0;
  bool in_order = true;

  std::jthread consumer([&] {
    long expect = 0;
    while (received < M)
      if (auto v = q.try_pop()) {
        if (*v != expect++) in_order = false;
        sum += *v;
        ++received;
      }
  });

  std::jthread producer([&] {
    for (long i = 0; i < M; ++i)
      while (!q.try_push(i)) {
      }  // spin while full
  });

  producer.join();
  consumer.join();

  EXPECT_TRUE(in_order);
  ASSERT_EQ(received, M);
  ASSERT_EQ(sum, M * (M - 1) / 2);
}

}  // namespace
