#https://cmake.org/pipermail/cmake/2007-August/015891.html


Set( MODULE_OUTPUT_FILES    accesio_pci.o
                            accesio_pci.mod.c
                            accesio_pci.mod.o
                            accesio_pci.ko
                            .accesio_pci.o.cmd
                            .accesio_pci.ko.cmd
                            .accesio_pci.mod.o.cmd
                            .tmp_versions/accesio_pci.mod
                            .tmp_versions/ )

Set( MODULE_SOURCE_FILES    accesio_pci.c )

Set( DRIVER_FILE        accesio_pci.ko )
Set( DRIVER_TARGET_NAME accesio_pci-module )
Set( DRIVER_BIN_FILE    ${PROJECT_BINARY_DIR}/${DRIVER_FILE} )
Set( MODULE_SOURCE_DIR  ${PROJECT_SOURCE_DIR}/module )

if (AIO_DEBUG)
  set (AIO_CFLAGS "-DAIO_DEBUG=1")
else()
  set (AIO_CFLAGS "-DAIO_DEBUG=0")
endif (AIO_DEBUG)

if (AIO_VERBOSE)
  set (AIO_CFLAGS "${AIO_CFLAGS} -DAIO_VERBOSE=1")
else()
  set (AIO_CFLAGS "${AIO_CFLAGS} -DAIO_VERBOSE=0")
endif(AIO_VERBOSE)

Set( KERNEL_DIR "/lib/modules/${CMAKE_SYSTEM_VERSION}/build" )
Set( KBUILD_CMD ${CMAKE_MAKE_PROGRAM}
                -C ${KERNEL_DIR}
                AIO_CFLAGS=${AIO_CFLAGS}
                M=${MODULE_SOURCE_DIR}
                modules )

Add_Custom_Command( OUTPUT  ${DRIVER_BIN_FILE}
                            ${MODULE_OUTPUT_FILES}
                    COMMAND ${KBUILD_CMD}
                    COMMAND cp -f ${DRIVER_FILE} ${PROJECT_BINARY_DIR}/module/${DRIVER_FILE}
                    WORKING_DIRECTORY ${MODULE_SOURCE_DIR}
                    DEPENDS ${MODULE_SOURCE_FILES} VERBATIM )

Add_Custom_Target ( ${DRIVER_TARGET_NAME}
                    ALL
                    DEPENDS ${DRIVER_BIN_FILE} )

set (KBUILD_INSTALL_CMD ${KBUILD_CMD} modules_install)
set (DEPMOD depmod -a)

install (CODE "execute_process(COMMAND ${KBUILD_INSTALL_CMD})"
         CODE "execute_process(COMMAND ${DEPMOD})")
