project("protoc-plugin-examples")
cmake_minimum_required(VERSION 3.17)

include(FindProtobuf)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake
)

find_package(Protobuf REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(Threads)

# ----------------------------------------------------------------------------
# Build protoc plugins and create a list of target names and protoc args for
# the protoc plugins.

add_subdirectory(plugins)

include(all_targets)
get_all_targets(PROTOC_PLUGIN_TARGETS)
# message("All targets: ${PROTOC_PLUGIN_TARGETS}")

foreach(target ${PROTOC_PLUGIN_TARGETS})
    get_target_property(plugin_name ${target} PROTOC_PLUGIN_NAME)
    get_target_property(plugin_protoc_gen_name ${target} PROTOC_GEN_NAME)
    get_target_property(plugin_path ${target} PROTOC_PLUGIN_PATH)

    list(APPEND PROTOC_PLUGIN_ARGS
        "--plugin=${plugin_name}=${plugin_path}"
        "--${plugin_protoc_gen_name}_out=${CMAKE_CURRENT_BINARY_DIR}")
endforeach(target ${PROTOC_PLUGIN_TARGETS})

# ----------------------------------------------------------------------------
# Generate c++ files from protos

set(PROTO_LIB_NAME "${PROJECT_NAME}-protos")
set(PROTO_DIR "protos")
set(PROTO_FILES
    ${PROTO_DIR}/options.proto
    ${PROTO_DIR}/foo.proto
)

foreach(proto ${PROTO_FILES})
    get_filename_component(PROTO_NAME "${proto}" NAME_WE)
    set(PROTO_HEADER "${PROTO_DIR}/${PROTO_NAME}.pb.h")
    set(PROTO_SRC    "${PROTO_DIR}/${PROTO_NAME}.pb.cc")

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        string(CONCAT cmd
            "protoc -I ${CMAKE_CURRENT_SOURCE_DIR} "
            "${CMAKE_CURRENT_SOURCE_DIR}/${proto} "
            "--cpp_out=${CMAKE_CURRENT_BINARY_DIR} "
            "${PROTOC_PLUGIN_ARGS}"
        )
        message(STATUS "${cmd}")
    endif()

    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_SRC}"
                "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_HEADER}"
        COMMAND LIBRARY_PATH=${Protobuf_LIBRARY} ${Protobuf_PROTOC_EXECUTABLE}
        ARGS -I ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/${proto}
            --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
            ${PROTOC_PLUGIN_ARGS}
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${proto}" ${PROTOC_PLUGIN_TARGETS}
        COMMENT "${proto} -> ${PROTO_SRC} ${PROTO_HEADER}"
    )

    list(APPEND SOURCES "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_SRC}")
    list(APPEND HEADERS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_HEADER}")
endforeach()

add_library(${PROTO_LIB_NAME} STATIC ${SOURCES} ${HEADERS})

target_compile_options(${PROTO_LIB_NAME} PRIVATE -std=c++11)

target_link_libraries(${PROTO_LIB_NAME} PRIVATE
    ${Protobuf_LIBRARIES} ${Protobuf_PROTOC_LIBRARIES})

target_include_directories(${PROTO_LIB_NAME} PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR} ${Protobuf_INCLUDE_DIR})

install(TARGETS ${PROTO_LIB_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/protos
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})


# -----------------------------------------------------------
# Make and install the example apps

add_subdirectory(examples)
