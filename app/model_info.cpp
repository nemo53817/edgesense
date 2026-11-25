// Subtask 1: print what the YOLOX-Nano model expects and returns.
//
//   cmake --build --preset debug && ./build/debug/app/model_info [path/to/model.onnx]
//
// Goal: answer these questions from the model itself, not from documentation:
//   1. How many inputs and outputs, and what are their names?
//   2. Shape and element type of each (expected: input 1x3x416x416 float, output 1x3549x85 float).
//   3. Bonus: run once on an all-zero input and print the first rows of the output. Are the box
//      values in pixels (hundreds), i.e. decoding is inside the model, or small raw offsets?

#include <onnxruntime_cxx_api.h>

#include <cstdlib>
#include <exception>
#include <iostream>

int main(int argc, char** argv) {
  const char* model_path = argc > 1 ? argv[1] : EDGESENSE_MODEL_PATH;
  std::cout << "model: " << model_path << '\n';

  try {
    // TODO 1: create the Ort::Env. Which object must outlive which?

    // TODO 2: Ort::SessionOptions + Ort::Session for model_path.

    // TODO 3: for each input i: name (GetInputNameAllocated; who owns the string?),
    //         shape and element type (GetInputTypeInfo(i).GetTensorTypeAndShapeInfo()).
    //         Note: a dimension of -1 means "dynamic". Does this model have any?

    // TODO 4: the same for outputs.

    // TODO 5 (bonus): allocate a zero-filled input buffer of the right size, wrap it with
    //         Ort::Value::CreateTensor (no copy), call session.Run, print the first rows.
  } catch (const std::exception& e) {  // Ort::Exception derives from std::exception
    std::cerr << "error: " << e.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
