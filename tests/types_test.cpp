#include <gtest/gtest.h>

#include <edgesense/core/types.hpp>
#include <type_traits>

namespace {

using edgesense::core::Box;
using edgesense::core::Detection;
using edgesense::core::Point;

// Compile-time guarantees the per-frame code relies on: plain aggregates that can live in
// preallocated arrays and be copied with memcpy, and helpers usable in constant expressions.
static_assert(std::is_aggregate_v<Point> && std::is_aggregate_v<Box> &&
              std::is_aggregate_v<Detection>);
static_assert(std::is_trivially_copyable_v<Point> && std::is_trivially_copyable_v<Box> &&
              std::is_trivially_copyable_v<Detection>);
static_assert(Box{{0, 0}, {4, 2}}.area() == 8.0f);

TEST(Point, DefaultIsOrigin) {
  const Point p;
  EXPECT_EQ(p.x, 0.0f);
  EXPECT_EQ(p.y, 0.0f);
}

TEST(Point, EqualityComparesBothCoordinates) {
  EXPECT_EQ((Point{1.5f, 2.0f}), (Point{1.5f, 2.0f}));
  EXPECT_NE((Point{1.5f, 2.0f}), (Point{1.5f, 3.0f}));
  EXPECT_NE((Point{1.5f, 2.0f}), (Point{0.5f, 2.0f}));
}

TEST(Box, DefaultIsEmptyAtOrigin) {
  const Box b;
  EXPECT_EQ(b.top_left, Point{});
  EXPECT_EQ(b.bottom_right, Point{});
  EXPECT_EQ(b.width(), 0.0f);
  EXPECT_EQ(b.height(), 0.0f);
  EXPECT_EQ(b.area(), 0.0f);
}

// Continuous coordinates: a box over pixels 10..19 spans x 10..20, so width is 10 (no +1).
TEST(Box, SizeUsesContinuousCoordinates) {
  const Box b{{10.0f, 20.0f}, {20.0f, 50.0f}};
  EXPECT_EQ(b.width(), 10.0f);
  EXPECT_EQ(b.height(), 30.0f);
  EXPECT_EQ(b.area(), 300.0f);
}

TEST(Box, FractionalCoordinates) {
  const Box b{{0.25f, 1.5f}, {2.75f, 3.0f}};
  EXPECT_FLOAT_EQ(b.width(), 2.5f);
  EXPECT_FLOAT_EQ(b.height(), 1.5f);
  EXPECT_FLOAT_EQ(b.area(), 3.75f);
}

TEST(Box, CenterIsMidpointOfCorners) {
  EXPECT_EQ((Box{{10.0f, 20.0f}, {30.0f, 60.0f}}.center()), (Point{20.0f, 40.0f}));
  EXPECT_EQ((Box{{-4.0f, -2.0f}, {4.0f, 2.0f}}.center()), (Point{0.0f, 0.0f}));
  EXPECT_EQ((Box{{1.0f, 1.0f}, {2.0f, 2.0f}}.center()), (Point{1.5f, 1.5f}));
}

TEST(Box, DegenerateBoxHasZeroArea) {
  const Box line{{5.0f, 5.0f}, {5.0f, 15.0f}};  // zero width
  EXPECT_EQ(line.width(), 0.0f);
  EXPECT_EQ(line.height(), 10.0f);
  EXPECT_EQ(line.area(), 0.0f);
}

// Documents current behaviour for inverted boxes (bottom_right left of / above top_left).
// Whether area() should clamp to 0 is decided with NMS (subtask 3); update this test then.
TEST(Box, InvertedBoxHasNegativeSize) {
  const Box inverted{{20.0f, 20.0f}, {10.0f, 30.0f}};
  EXPECT_EQ(inverted.width(), -10.0f);
  EXPECT_EQ(inverted.height(), 10.0f);
  EXPECT_EQ(inverted.area(), -100.0f);
}

TEST(Box, EqualityComparesBothCorners) {
  const Box a{{1.0f, 2.0f}, {3.0f, 4.0f}};
  EXPECT_EQ(a, (Box{{1.0f, 2.0f}, {3.0f, 4.0f}}));
  EXPECT_NE(a, (Box{{1.0f, 2.0f}, {3.0f, 5.0f}}));
  EXPECT_NE(a, (Box{{0.0f, 2.0f}, {3.0f, 4.0f}}));
}

TEST(Detection, DefaultIsUnsetWithZeroScore) {
  const Detection d;
  EXPECT_EQ(d.box, Box{});
  EXPECT_EQ(d.class_id, -1);
  EXPECT_EQ(d.score, 0.0f);
}

TEST(Detection, AggregateInitialization) {
  const Detection d{{{10.0f, 20.0f}, {30.0f, 40.0f}}, 2, 0.75f};
  EXPECT_EQ(d.box, (Box{{10.0f, 20.0f}, {30.0f, 40.0f}}));
  EXPECT_EQ(d.class_id, 2);
  EXPECT_EQ(d.score, 0.75f);
}

TEST(Detection, EqualityComparesEveryField) {
  const Detection base{{{0.0f, 0.0f}, {1.0f, 1.0f}}, 0, 0.5f};
  Detection other = base;
  EXPECT_EQ(base, other);

  other.box.bottom_right.x = 2.0f;
  EXPECT_NE(base, other);
  other = base;
  other.class_id = 1;
  EXPECT_NE(base, other);
  other = base;
  other.score = 0.6f;
  EXPECT_NE(base, other);
}

}  // namespace
