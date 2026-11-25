# Every app/*.cpp becomes one executable with the same name, linked against edgesense,
# ONNX Runtime and OpenCV. EDGESENSE_MODEL_PATH is passed as a compile-time default.
file(GLOB EDGESENSE_APP_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

foreach(src IN LISTS EDGESENSE_APP_SOURCES)
  get_filename_component(app ${src} NAME_WE)
  add_executable(${app} ${src})
  target_link_libraries(${app} PRIVATE edgesense::edgesense edgesense_warnings
                                       onnxruntime::onnxruntime ${OpenCV_LIBS})
  target_compile_definitions(${app} PRIVATE EDGESENSE_MODEL_PATH="${EDGESENSE_MODEL_PATH}")
endforeach()
