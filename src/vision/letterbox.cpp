#include <edgesense/vision/letterbox.hpp>

namespace edgesense::vision {

// Notes for the implementation (subtask 2):
//
// 1. Scale and placement: scale = min(T / w, T / h); new size = round(w * scale), round(h * scale).
//    The YOLOX reference preprocessing places the resized image at the TOP-LEFT of the canvas and
//    fills the rest with 114 (gray). Centered placement is also common (other YOLO versions).
//    The model works best with what it was trained with, so prefer YOLOX's choice; whichever you
//    pick, LetterboxInfo must describe it so map_to_source stays exact.
//
// 2. Resizing without OpenCV (our rule: no OpenCV in the library). For each destination pixel
//    (x, y) find the source position (x / scale, y / scale) and sample it:
//      - nearest neighbour: take the closest source pixel. Simple, fine to start with.
//      - bilinear: weighted average of the 4 surrounding pixels. Smoother; what cv::INTER_LINEAR
//        does, and what YOLOX used in training. Watch the pixel-center convention:
//        src = (dst + 0.5) / scale - 0.5 is the usual one.
//    Doing this yourself is the learning part, and it is exactly the loop we will speed up with
//    NEON on the Pi in M4.
//
// 3. Layout and values: read HWC uint8 through the stride, write CHW float:
//      dst[c * T * T + y * T + x]
//    Channel order and scaling: does the model expect BGR or RGB? Raw 0..255 or 0..1 / normalized?
//    Not documented in the .onnx file. Pick the YOLOX reference behaviour, then confirm it in
//    subtask 6: wrong choices still "work" but give visibly worse detections.

LetterboxInfo letterbox(const core::ImageView& /*src*/, int /*target_size*/,
                        std::span<float> /*dst*/) {
  // TODO(subtask 2): implement.
  return {};
}

core::Box map_to_source(const core::Box& /*box*/, const LetterboxInfo& /*info*/) {
  // TODO(subtask 2): implement.
  return {};
}

}  // namespace edgesense::vision
