project(QmlFormBuilder LANGUAGES C CXX)
cmake_minimum_required(VERSION 3.10)

include(FindProtobuf)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------

find_package(Protobuf REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(Threads)
find_package(Qt5
    COMPONENTS
        Core
        Qml
        Quick
        QuickControls2
        QuickWidgets
    REQUIRED
)

# -----------------------------------------------------------------------------
# Protos
# -----------------------------------------------------------------------------
# Generate the "core" c++ files from protos
# These don't use the custom protoc plugins

set(PROTO_INTERNAL_LIB_NAME "${PROJECT_NAME}-internal-protos")
set(PROTO_INTERNAL_DIR "protos/options")
list(APPEND PROTO_INTERNAL_FILES ${PROTO_INTERNAL_DIR}/options.proto ${PROTO_INTERNAL_DIR}/validate.proto)

foreach(proto ${PROTO_INTERNAL_FILES})
    get_filename_component(PROTO_NAME "${proto}" NAME_WE)
    get_filename_component(PROTO_DIR "${proto}" DIRECTORY)

    set(PROTO_HEADER "${PROTO_DIR}/${PROTO_NAME}.pb.h")
    set(PROTO_SRC    "${PROTO_DIR}/${PROTO_NAME}.pb.cc")

    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_SRC}"
               "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_HEADER}"
        COMMAND LIBRARY_PATH=${Protobuf_LIBRARY} ${Protobuf_PROTOC_EXECUTABLE}
        ARGS -I ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/${proto}
            --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
            --experimental_allow_proto3_optional
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${proto} ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_INTERNAL_DIR}
        COMMENT "${proto} -> ${PROTO_SRC} ${PROTO_HEADER}"
    )

    list(APPEND INTERNAL_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_SRC}")
    list(APPEND INTERNAL_HEADERS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_HEADER}")
endforeach()

add_library(${PROTO_INTERNAL_LIB_NAME} STATIC
    ${INTERNAL_SOURCES}
    ${INTERNAL_HEADERS}
)

target_link_libraries(${PROTO_INTERNAL_LIB_NAME}
    PRIVATE
        ${Protobuf_LIBRARIES}
        ${Protobuf_PROTOC_LIBRARIES}
)

target_include_directories(${PROTO_INTERNAL_LIB_NAME}
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
        ${Protobuf_INCLUDE_DIR}
)

install(TARGETS ${PROTO_INTERNAL_LIB_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/protos DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})


# ----------------------------------------------------------------------------
# Build protoc plugins and create a list of target names and protoc args for
# the protoc plugins.

add_subdirectory(plugins)
include(all_targets)
get_all_targets(PROTOC_PLUGIN_TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/plugins)

foreach(target ${PROTOC_PLUGIN_TARGETS})
    get_target_property(plugin_name ${target} PROTOC_PLUGIN_NAME)
    get_target_property(plugin_protoc_gen_name ${target} PROTOC_GEN_NAME)
    get_target_property(plugin_path ${target} PROTOC_PLUGIN_PATH)

    list(APPEND PROTOC_PLUGIN_ARGS
        "--plugin=${plugin_name}=${plugin_path}"
        "--${plugin_protoc_gen_name}_out=${CMAKE_CURRENT_BINARY_DIR}")
endforeach(target ${PROTOC_PLUGIN_TARGETS})


# ----------------------------------------------------------------------------
# Generate the "api" c++ files from protos
# These use the custom protoc plugins

set(PROTO_API_LIB_NAME "${PROJECT_NAME}-api-protos")
set(PROTO_API_DIR "protos/api")
set(PROTO_API_FILES
    ${PROTO_API_DIR}/foo.proto
    ${PROTO_API_DIR}/bar.proto
    ${PROTO_API_DIR}/baz.proto
    ${PROTO_API_DIR}/platform.proto
    ${PROTO_API_DIR}/kitchen_sink.proto
)

foreach(proto ${PROTO_API_FILES})
    get_filename_component(PROTO_NAME "${proto}" NAME_WE)
    get_filename_component(PROTO_DIR "${proto}" DIRECTORY)

    set(PROTO_HEADER "${PROTO_DIR}/${PROTO_NAME}.pb.h")
    set(PROTO_SRC    "${PROTO_DIR}/${PROTO_NAME}.pb.cc")

    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_SRC}"
               "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_HEADER}"
        COMMAND LIBRARY_PATH=${Protobuf_LIBRARY} ${Protobuf_PROTOC_EXECUTABLE}
        ARGS -I ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/${proto}
            --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
            ${PROTO_INTERNAL_FILES}
            ${PROTOC_PLUGIN_ARGS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${proto} ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_API_DIR} ${PROTOC_PLUGIN_TARGETS}
        COMMENT "${proto} -> ${PROTO_SRC} ${PROTO_HEADER}"
    )

    list(APPEND API_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_SRC}")
    list(APPEND API_HEADERS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_HEADER}")
endforeach()

add_library(${PROTO_API_LIB_NAME} STATIC
    ${API_SOURCES}
    ${API_HEADERS}
)

target_link_libraries(${PROTO_API_LIB_NAME}
    PUBLIC
        ${PROTO_INTERNAL_LIB_NAME}
    PRIVATE
        ${Protobuf_LIBRARIES}
        ${Protobuf_PROTOC_LIBRARIES}
)

target_include_directories(${PROTO_API_LIB_NAME}
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
        ${Protobuf_INCLUDE_DIR}
)

install(TARGETS ${PROTO_API_LIB_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/protos DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/forms DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# -----------------------------------------------------------------------------
# C++/Qt Source code
# -----------------------------------------------------------------------------

add_executable(${PROJECT_NAME}
    json_reader.h
    main.cc
    qml.qrc
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt5::Core
        Qt5::Qml
        Qt5::Quick
        Qt5::QuickControls2
        Qt5::QuickWidgets
    PUBLIC
        ${Protobuf_LIBRARIES}
        ${PROTO_INTERNAL_LIB_NAME}
        ${PROTO_API_LIB_NAME}
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY examples DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})