#pragma once

// Letterbox: fit an image of any size into the model's square input without distorting it.
//
//   original 1280x720 ─► scale = min(416/1280, 416/720) = 0.325 ─► resized 416x234
//                     ─► placed into a 416x416 canvas, the rest filled with a constant color
//
// Plain resizing to 416x416 would squash the image and the model would see distorted objects.
// Because the scale is uniform, every box the model returns can be mapped back exactly:
// subtract the padding offset, divide by the scale. LetterboxInfo carries what that needs.
//
// Output layout: the model wants float CHW (all blue values, then all green, then all red),
// while images are stored HWC (B,G,R,B,G,R,...). Letterbox writes CHW directly into a buffer the
// caller allocated once, so no per-frame allocation happens here.

#include <edgesense/core/image_view.hpp>
#include <edgesense/core/types.hpp>
#include <span>

namespace edgesense::vision {

// TODO(subtask 2): fields needed to undo the letterbox: the scale, and where the resized image
// was placed in the canvas (pad / offset in x and y). Float or int for each?
struct LetterboxInfo {};

// Resize `src` (keeping aspect ratio) into a target_size x target_size canvas, written as float
// CHW into `dst` (size 3 * target_size * target_size). Returns how to map coordinates back.
//
// TODO(subtask 2): decide and document the contract:
//   - what if dst.size() is wrong, or src is empty? (assert? return an error? throw?)
//   - channel order of the output (the model's training decides; see letterbox.cpp notes)
//   - pad value, and where the image goes in the canvas (centered or top-left)
// The signature is a suggestion; change it if your design needs something else.
LetterboxInfo letterbox(const core::ImageView& src, int target_size, std::span<float> dst);

// Map a box from model-input coordinates (the 416x416 canvas) back to the original image.
// TODO(subtask 2): should the result be clamped to the image bounds?
core::Box map_to_source(const core::Box& box, const LetterboxInfo& info);

}  // namespace edgesense::vision
