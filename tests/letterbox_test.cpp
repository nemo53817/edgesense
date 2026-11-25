#include <gtest/gtest.h>

#include <edgesense/vision/letterbox.hpp>

// Test ideas for subtask 2 (pure geometry, use tiny synthetic images built in the test):
//
// Scale and padding (LetterboxInfo):
//   - wide image (e.g. 8x4 -> T=4): scale 0.5, padding only in y
//   - tall image, square image (no padding), image already T x T (scale 1)
//   - upscaling a small image (2x2 -> T=4): scale 2
//
// Pixel content:
//   - uniform color image: the image area equals that color, the pad area equals the pad value
//   - channel order and CHW layout: a single-pixel image with distinct B, G, R values
//   - stride: an ImageView with padded rows (stride > width * 3) gives the same result as a
//     contiguous one; garbage in the row padding must not leak into the output
//
// Coordinates:
//   - map_to_source round trip: a box in the original image, scaled and offset forward by hand,
//     maps back to the original (EXPECT_NEAR, not EXPECT_EQ, for floats)
//   - corner cases: box touching the pad area, clamping (if you chose to clamp)
//
// Contract:
//   - wrong dst size / empty image behaves as documented in letterbox.hpp
