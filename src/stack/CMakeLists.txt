add_library (diametercodec-stack
    ${CMAKE_CURRENT_LIST_DIR}/Vendor.cpp
)

target_include_directories (diametercodec-stack
  PUBLIC ${ERT_DIAMETERCODEC_INCLUDE_BUILD_DIR}
)
