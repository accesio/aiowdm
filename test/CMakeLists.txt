include_directories(${AIOWDM_SOURCE_DIR}/include)

add_compile_definitions( AIO_DEBUG=$<CONFIG:Debug>)

if (AIO_VERBOSE)
  add_compile_definitions(AIO_VERBOSE=1)
else()
  add_compile_definitions(AIO_VERBOSE=0)
endif(AIO_VERBOSE)


add_executable(query query.cpp)
target_link_libraries(query $<TARGET_OBJECTS:aiowdm>)

add_executable(RegisterIo RegisterIo.cpp)
target_link_libraries(RegisterIo $<TARGET_OBJECTS:aiowdm>)

add_executable(pcie-adio16-16fds-irq pcie-adio16-16fds-irq.cpp)
target_link_libraries(pcie-adio16-16fds-irq $<TARGET_OBJECTS:aiowdm>)
