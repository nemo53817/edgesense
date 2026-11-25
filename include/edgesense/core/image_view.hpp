#pragma once

// Non-owning view of an image in memory: the library's way to receive pixels without depending
// on OpenCV (apps wrap a cv::Mat, the Pi camera path will wrap its frame buffer). Copying an
// ImageView copies a pointer and a few ints, never pixels.

namespace edgesense::core {

// TODO(subtask 2): pixel format enum. Which formats do you need now (OpenCV reads BGR), and which
// later (cameras often give RGB, YUV/NV12)? An `enum class` with an explicit small underlying type
// is a common choice.

// TODO(subtask 2): fields. Things to decide:
//   - data: pointer to the first byte of the first row. Const? (The library only reads frames.)
//   - width, height in pixels. Signed or unsigned? (Mixing with size_t math under -Wconversion.)
//   - stride (a.k.a. step / pitch): bytes from the start of one row to the start of the next.
//     It can be LARGER than width * channels (row padding for alignment, or a view into a crop
//     of a bigger image). Never assume rows are contiguous; cv::Mat::step gives you this value.
//   - format, and from it the number of channels / bytes per pixel.
//   - How does a reader find pixel (x, y), channel c? Write the formula in a comment here.
struct ImageView {};

}  // namespace edgesense::core
