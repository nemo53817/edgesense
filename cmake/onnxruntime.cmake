# Provides the imported target onnxruntime::onnxruntime (prebuilt ONNX Runtime, CPU).
#
# By default the archive for the current platform is downloaded from this repo's
# "onnxruntime-<version>" release, a verified copy of Microsoft's NuGet package made by
# scripts/repackage-onnxruntime.sh. Set EDGESENSE_ONNXRUNTIME_ROOT to use a local install
# instead (a directory with include/ and lib/).
set(EDGESENSE_ONNXRUNTIME_VERSION 1.32.0)
set(EDGESENSE_ONNXRUNTIME_ROOT "" CACHE PATH "Local ONNX Runtime (include/ + lib/); empty = download")

if(NOT EDGESENSE_ONNXRUNTIME_ROOT)
  if(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
    set(_ort_platform osx-arm64)
    set(_ort_sha256 46f1541516ff3f16df0874b5368bee3b8e5604ee4688adbef970af1ae4a354a7)
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")
    set(_ort_platform linux-x64)
    set(_ort_sha256 a02d26be216f724ca6e9f2731fb28842158e0aeae10f49c11437a11fcd48311f)
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$")
    set(_ort_platform linux-aarch64)
    set(_ort_sha256 0c5499a011eb4e48d732f969fe5d9e68ee0f9aecb1995b3423e60a15e19c79b9)
  else()
    message(FATAL_ERROR "No prebuilt ONNX Runtime for ${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}; "
                        "set EDGESENSE_ONNXRUNTIME_ROOT")
  endif()

  include(FetchContent)
  set(_ort_name onnxruntime-${_ort_platform}-${EDGESENSE_ONNXRUNTIME_VERSION})
  FetchContent_Declare(
    onnxruntime
    URL https://github.com/nemo53817/edgesense/releases/download/onnxruntime-${EDGESENSE_ONNXRUNTIME_VERSION}/${_ort_name}.tgz
    URL_HASH SHA256=${_ort_sha256}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  FetchContent_MakeAvailable(onnxruntime)  # no CMakeLists.txt inside: download + extract only
  set(EDGESENSE_ONNXRUNTIME_ROOT ${onnxruntime_SOURCE_DIR} CACHE PATH "" FORCE)
endif()

# Versioned file + the name the loader looks for (install name / SONAME).
if(APPLE)
  set(_ort_file libonnxruntime.${EDGESENSE_ONNXRUNTIME_VERSION}.dylib)
  set(_ort_soname @rpath/libonnxruntime.1.dylib)
else()
  set(_ort_file libonnxruntime.so.${EDGESENSE_ONNXRUNTIME_VERSION})
  set(_ort_soname libonnxruntime.so.1)
endif()

add_library(onnxruntime::onnxruntime SHARED IMPORTED GLOBAL)
set_target_properties(
  onnxruntime::onnxruntime
  PROPERTIES IMPORTED_LOCATION ${EDGESENSE_ONNXRUNTIME_ROOT}/lib/${_ort_file}
             IMPORTED_SONAME ${_ort_soname}
             INTERFACE_INCLUDE_DIRECTORIES ${EDGESENSE_ONNXRUNTIME_ROOT}/include)
message(STATUS "edgesense: ONNX Runtime ${EDGESENSE_ONNXRUNTIME_VERSION} at ${EDGESENSE_ONNXRUNTIME_ROOT}")
