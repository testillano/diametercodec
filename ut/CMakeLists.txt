add_executable( unit-test "")

add_subdirectory(core)
add_subdirectory(stack)

# To avoid setting absolute paths for libraries down at 'target_link_libraries':
add_library(ert_logger STATIC IMPORTED)
#add_library(ert_metrics STATIC IMPORTED)
#add_library(prometheus-cpp-pull STATIC IMPORTED)
#add_library(prometheus-cpp-core STATIC IMPORTED)
#add_library(boost_system STATIC IMPORTED)
#add_library(nlohmann_json_schema_validator STATIC IMPORTED)

set_property(TARGET ert_logger PROPERTY IMPORTED_LOCATION ${CMAKE_PREFIX_PATH}/lib/ert/libert_logger.a)
#set_property(TARGET ert_metrics PROPERTY IMPORTED_LOCATION ${CMAKE_PREFIX_PATH}/lib/ert/libert_metrics.a)
#set_property(TARGET prometheus-cpp-pull PROPERTY IMPORTED_LOCATION ${CMAKE_PREFIX_PATH}/lib/libprometheus-cpp-pull.a)
#set_property(TARGET prometheus-cpp-core PROPERTY IMPORTED_LOCATION ${CMAKE_PREFIX_PATH}/lib/libprometheus-cpp-core.a)
#set_property(TARGET boost_system PROPERTY IMPORTED_LOCATION ${CMAKE_PREFIX_PATH}/lib/libboost_system.a)
#set_property(TARGET nlohmann_json_schema_validator PROPERTY IMPORTED_LOCATION ${CMAKE_PREFIX_PATH}/lib/libnlohmann_json_schema_validator.a)

target_link_libraries( unit-test
PRIVATE
${CMAKE_EXE_LINKER_FLAGS}
        gtest
        gmock
        gtest_main

        ert_diametercodec

        ert_logger
#        ert_metrics

        prometheus-cpp-pull
        prometheus-cpp-core
        z

#        boost_system   #Needed by nghttp2_asio
#        ssl            #Needed by boost_system
#        crypto         #Needed by ssl, and need to be appended after ssl

        nlohmann_json_schema_validator

)

