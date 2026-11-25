// Prints what an ONNX model expects and returns, then runs it once on an all-zero input.
//
//   cmake --build --preset debug && ./build/debug/app/model_info [path/to/model.onnx]
//
// For YOLOX-Nano this answers: input/output names, shapes and element types, and whether the
// box values in the output are already decoded to pixels (large) or raw grid offsets (small).

#include <onnxruntime_cxx_api.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

// Everything we need to know about one model input or output, copied out of ORT objects so it
// stays valid after the Ort::TypeInfo it came from is gone.
struct TensorDesc {
  std::string name;
  ONNXTensorElementDataType type = ONNX_TENSOR_ELEMENT_DATA_TYPE_UNDEFINED;
  std::vector<int64_t> shape;  // -1 = dynamic dimension
};

const char* typeName(ONNXTensorElementDataType t) {
  switch (t) {
    case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT:
      return "float";
    case ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8:
      return "uint8";
    case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64:
      return "int64";
    case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16:
      return "float16";
    default:
      return "other";
  }
}

std::string shapeToString(const std::vector<int64_t>& shape) {
  std::string s = "[";
  for (std::size_t i = 0; i < shape.size(); ++i) {
    if (i > 0) s += ", ";
    s += std::to_string(shape[i]);
  }
  return s + "]";
}

void print(const char* kind, std::size_t i, const TensorDesc& d) {
  std::cout << kind << ' ' << i << ": name=" << d.name << "  type=" << typeName(d.type) << '('
            << static_cast<int>(d.type) << ")  shape=" << shapeToString(d.shape) << '\n';
}

// Name: the AllocatedStringPtr owns the string and frees it at the end of this function, so we
// copy it. Type/shape: the tensor info is a view into typeInfo, so typeInfo must be a named
// variable that outlives it (a temporary would leave a dangling view).
template <class GetName, class GetTypeInfo>
std::vector<TensorDesc> describe(std::size_t count, GetName getName, GetTypeInfo getTypeInfo) {
  Ort::AllocatorWithDefaultOptions allocator;  // owned by ORT; this is only a handle to it
  std::vector<TensorDesc> result;
  for (std::size_t i = 0; i < count; ++i) {
    const Ort::AllocatedStringPtr name = getName(i, allocator);
    const Ort::TypeInfo typeInfo = getTypeInfo(i);
    if (typeInfo.GetONNXType() != ONNX_TYPE_TENSOR) {
      result.push_back({name.get(), ONNX_TENSOR_ELEMENT_DATA_TYPE_UNDEFINED, {}});
      continue;  // sequences/maps have no tensor shape
    }
    const auto info = typeInfo.GetTensorTypeAndShapeInfo();
    result.push_back({name.get(), info.GetElementType(), info.GetShape()});
  }
  return result;
}

// Runs the model once on zeros and prints a summary of a [1, rows, cols] float output.
void runOnZeros(Ort::Session& session, const std::vector<TensorDesc>& inputs,
                const std::vector<TensorDesc>& outputs) {
  if (inputs.size() != 1 || inputs[0].type != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
    std::cout << "zero run skipped: expected exactly one float input\n";
    return;
  }

  // 1. Concrete input shape. A dynamic dimension (-1) has to be chosen by us; use 1.
  std::vector<int64_t> shape = inputs[0].shape;
  std::size_t elementCount = 1;
  for (auto& dim : shape) {
    if (dim < 0) dim = 1;
    elementCount *= static_cast<std::size_t>(dim);
  }

  // 2. We own the input memory. CreateTensor does NOT copy: the Ort::Value only points at
  //    inputData, so inputData must stay alive (and unmoved) until Run returns. In the real
  //    pipeline this buffer is allocated once and reused for every frame.
  std::vector<float> inputData(elementCount, 0.0f);
  const Ort::MemoryInfo memoryInfo =
      Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  const Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
      memoryInfo, inputData.data(), inputData.size(), shape.data(), shape.size());

  // 3. Run takes C arrays of names. The const char* point into our std::strings, which outlive
  //    the call. Outputs are allocated by ORT and owned by the returned Ort::Values.
  std::vector<const char*> inputNames{inputs[0].name.c_str()};
  std::vector<const char*> outputNames;
  for (const auto& o : outputs) outputNames.push_back(o.name.c_str());

  std::vector<Ort::Value> results =
      session.Run(Ort::RunOptions{nullptr}, inputNames.data(), &inputTensor, inputNames.size(),
                  outputNames.data(), outputNames.size());

  // 4. Read the first output. Its real shape comes from the result, not from the model metadata
  //    (metadata may contain -1; the result never does).
  const auto outShape = results[0].GetTensorTypeAndShapeInfo().GetShape();
  if (outShape.size() != 3) {
    std::cout << "zero run: output shape " << shapeToString(outShape) << ", not [1, rows, cols]\n";
    return;
  }
  const auto rows = static_cast<std::size_t>(outShape[1]);
  const auto cols = static_cast<std::size_t>(outShape[2]);
  const float* data = results[0].GetTensorData<float>();  // valid while results[0] lives

  // YOLOX row layout: cx, cy, w, h, objectness, then one score per class.
  std::cout << "\nzero input -> output " << shapeToString(outShape) << '\n'
            << std::fixed << std::setprecision(3);
  auto printRow = [&](std::size_t r) {
    const float* row = data + r * cols;
    const float* best = std::max_element(row + 5, row + cols);
    std::cout << "  row " << std::setw(4) << r << ": cx=" << row[0] << " cy=" << row[1]
              << " w=" << row[2] << " h=" << row[3] << " obj=" << row[4]
              << " best_class=" << (best - (row + 5)) << " (" << *best << ")\n";
  };
  for (std::size_t r = 0; r < 3; ++r) printRow(r);
  printRow(rows - 1);

  // The range of each box column over all rows tells pixels (~0..416) from raw offsets (~0..1).
  const char* columnNames[] = {"cx", "cy", "w", "h"};
  for (std::size_t c = 0; c < 4; ++c) {
    float lo = std::numeric_limits<float>::max();
    float hi = std::numeric_limits<float>::lowest();
    for (std::size_t r = 0; r < rows; ++r) {
      lo = std::min(lo, data[r * cols + c]);
      hi = std::max(hi, data[r * cols + c]);
    }
    std::cout << "  " << columnNames[c] << " range: [" << lo << ", " << hi << "]\n";
  }
}

}  // namespace

int main(int argc, char** argv) {
  const char* modelPath = argc > 1 ? argv[1] : EDGESENSE_MODEL_PATH;
  std::cout << "model: " << modelPath << '\n';

  try {
    // Destruction runs in reverse order: session first, env last. The env must outlive every
    // session created from it.
    Ort::Env env;
    Ort::SessionOptions sessionOptions;
    Ort::Session session(env, modelPath, sessionOptions);

    const auto inputs = describe(
        session.GetInputCount(),
        [&](std::size_t i, OrtAllocator* a) { return session.GetInputNameAllocated(i, a); },
        [&](std::size_t i) { return session.GetInputTypeInfo(i); });
    const auto outputs = describe(
        session.GetOutputCount(),
        [&](std::size_t i, OrtAllocator* a) { return session.GetOutputNameAllocated(i, a); },
        [&](std::size_t i) { return session.GetOutputTypeInfo(i); });

    for (std::size_t i = 0; i < inputs.size(); ++i) print("input ", i, inputs[i]);
    for (std::size_t i = 0; i < outputs.size(); ++i) print("output", i, outputs[i]);

    runOnZeros(session, inputs, outputs);
  } catch (const std::exception& e) {  // Ort::Exception derives from std::exception
    std::cerr << "error: " << e.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
