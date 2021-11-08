set(PLUGIN_PROTOC_GEN_NAME "basic-insertions")
set(PLUGIN_TARGET_NAME "protoc-gen-${PLUGIN_PROTOC_GEN_NAME}")

message(STATUS "Adding ${PLUGIN_TARGET_NAME}")

find_package(Protobuf REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(Threads)

add_executable(${PLUGIN_TARGET_NAME}
    generator.cc
    main.cc
)

set_target_properties(${PLUGIN_TARGET_NAME}
    PROPERTIES
        PROTOC_GEN_NAME "${PLUGIN_PROTOC_GEN_NAME}"
        PROTOC_PLUGIN_NAME "${PLUGIN_TARGET_NAME}"
        PROTOC_PLUGIN_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_TARGET_NAME}"
)

target_include_directories(${PLUGIN_TARGET_NAME}
    PRIVATE ${Protobuf_INCLUDE_DIR}
)

target_compile_options(${PLUGIN_TARGET_NAME}
    PRIVATE
        -Wall -Wextra -Wshadow -Wconversion
        -fdiagnostics-color=always
)

target_link_libraries(${PLUGIN_TARGET_NAME}
    PRIVATE
        ${Protobuf_PROTOC_LIBRARIES}
        ${Protobuf_LIBRARIES}
)
