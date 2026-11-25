# Downloads the YOLOX-Nano ONNX model (Apache-2.0, Megvii) into models/ once, verified by SHA256.
# Exposes EDGESENSE_MODEL_PATH for apps and tests.
set(EDGESENSE_MODEL_URL
    https://github.com/Megvii-BaseDetection/YOLOX/releases/download/0.1.1rc0/yolox_nano.onnx)
set(EDGESENSE_MODEL_SHA256 c789161ed43c8269fcd4e67c67eeeb4e80c622da2eb296a20bc6007bd18a0b7d)
set(EDGESENSE_MODEL_PATH ${PROJECT_SOURCE_DIR}/models/yolox_nano.onnx)

set(_have_model FALSE)
if(EXISTS ${EDGESENSE_MODEL_PATH})
  file(SHA256 ${EDGESENSE_MODEL_PATH} _model_sha)
  if(_model_sha STREQUAL EDGESENSE_MODEL_SHA256)
    set(_have_model TRUE)
  endif()
endif()

if(NOT _have_model)
  message(STATUS "edgesense: downloading ${EDGESENSE_MODEL_URL}")
  file(DOWNLOAD ${EDGESENSE_MODEL_URL} ${EDGESENSE_MODEL_PATH}
       EXPECTED_HASH SHA256=${EDGESENSE_MODEL_SHA256} STATUS _status TLS_VERIFY ON)
  list(GET _status 0 _code)
  if(NOT _code EQUAL 0)
    file(REMOVE ${EDGESENSE_MODEL_PATH})
    message(FATAL_ERROR "edgesense: model download failed: ${_status}")
  endif()
endif()
