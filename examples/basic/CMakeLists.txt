set(BINARY_NAME "basic")

add_executable(${BINARY_NAME}
    main.cc
)

target_link_libraries(${BINARY_NAME}
    PUBLIC
        ${Protobuf_LIBRARIES}
        ${PROTO_LIB_NAME}
)

install(TARGETS ${BINARY_NAME}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
