include_directories(${AIOWDM_SOURCE_DIR}/include)

add_executable(query query.cpp)
target_link_libraries(query $<TARGET_OBJECTS:aiowdm>)
