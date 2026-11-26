#pragma once

// Basic value types shared by all modules (vision, postprocess, inference, detector).
// Keep this header dependency-free: no OpenCV, no ONNX Runtime.
//
// Coordinates are continuous pixels: x grows to the right, y grows down, and a box covering
// pixels 10..19 has x from 10 to 20 (width = 20 - 10, no +1).
// Coordinate spaces: a Box does not record which image it is measured on. Functions that take
// or return boxes say so in their comment:
//   - model space: the model input canvas (e.g. 416x416 after letterbox), padding included.
//   - image space: the original image given to the detector.
// vision::map_to_source is the one place that converts model space to image space.

namespace edgesense::core {

struct Point {
  float x = 0.0f;
  float y = 0.0f;

  [[nodiscard]]
  constexpr bool operator==(const Point&) const noexcept = default;
};

// Axis-aligned box given by its corners. Expected: top_left.x <= bottom_right.x and
// top_left.y <= bottom_right.y; for an inverted box, width()/height() are negative.
struct Box {
  Point top_left;
  Point bottom_right;

  [[nodiscard]]
  constexpr Point center() const noexcept {
    return Point{(top_left.x + bottom_right.x) / 2, (top_left.y + bottom_right.y) / 2};
  }

  [[nodiscard]]
  constexpr float width() const noexcept {
    return bottom_right.x - top_left.x;
  }

  [[nodiscard]]
  constexpr float height() const noexcept {
    return bottom_right.y - top_left.y;
  }

  [[nodiscard]]
  constexpr float area() const noexcept {
    return width() * height();
  }

  [[nodiscard]]
  constexpr bool operator==(const Box&) const noexcept = default;
};

// One detected object.
struct Detection {
  Box box;             // space depends on the stage that produced it (see above)
  int class_id = -1;   // index into the COCO class list (0 = person, ...); -1 = unset
  float score = 0.0f;  // final confidence in [0, 1]; for YOLOX: objectness * class probability

  [[nodiscard]]
  constexpr bool operator==(const Detection&) const noexcept = default;
};

}  // namespace edgesense::core
