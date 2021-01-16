include_directories(${AIOWDM_SOURCE_DIR}/include)
add_library(aiowdm STATIC aiowdm.c)
add_library(aiowdm_shared SHARED aiowdm.c)

set_property(TARGET aiowdm aiowdm_shared PROPERTY POSITION_INDEPENDENT_CODE ON)


#if (AIOUSB_DEBUG)
#  target_compile_definitions (aiousb PRIVATE AIOUSB_DEBUG=1)
#  target_compile_definitions (aiousb_shared PRIVATE AIOUSB_DEBUG=1)
#else ()
#  target_compile_definitions (aiousb PRIVATE AIOUSB_DEBUG=0)
#  target_compile_definitions (aiousb_shared PRIVATE AIOUSB_DEBUG=0)
#endif (AIOUSB_DEBUG)

ADD_CUSTOM_COMMAND(TARGET aiowdm_shared POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E rename
                    $<TARGET_FILE:aiowdm_shared>
                    libaiowdm.so)

set (LIBRARY_FILES ${CMAKE_CURRENT_BINARY_DIR}/libaiousb.so ${CMAKE_CURRENT_BINARY_DIR}/libaiousb.a)

install (FILES ${LIBRARY_FILES} DESTINATION ${CMAKE_INSTALL_LIBDIR})

