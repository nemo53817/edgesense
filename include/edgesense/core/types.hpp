#pragma once

// Basic value types shared by all modules (vision, postprocess, inference, detector).
// Keep this header dependency-free: no OpenCV, no ONNX Runtime.

namespace edgesense::core {

// Axis-aligned bounding box.
// TODO(subtask 1): choose the representation and write it down here:
//   - corners (x1, y1, x2, y2) or center + size (cx, cy, w, h)? NMS and drawing prefer one,
//     YOLOX output gives you the other; where should the conversion happen?
//   - which coordinate space: model input (416x416, letterboxed) or original image? Maybe both
//     at different stages; does the type need to say which?
//   - float or int? (Think about the reverse letterbox mapping and IoU.)
struct Box {};

// One detected object.
// TODO(subtask 1): fields for the box, the class and the confidence.
//   - class id type: YOLOX-Nano has 80 COCO classes; what type fits, and what does -1 mean?
//   - score: is it objectness, class probability, or their product? (See YOLOX decode, subtask 5.)
struct Detection {};

}  // namespace edgesense::core
