cmake_minimum_required(VERSION 3.15)


message(STATUS "Generator is set to: ${CMAKE_GENERATOR}")
#set(CMAKE_VERBOSE_MAKEFILE ON)


set(DAEMON_NAME          wsdd)
set(DAEMON_MAJOR_VERSION 3)
set(DAEMON_MINOR_VERSION 0)
set(DAEMON_PATCH_VERSION patch)
# variants: patch - set PATCH_VERSION == number of commits(patches) between head and last tag
# variants: xxx   - set PATCH_VERSION == xxx (your variant)
set(DAEMON_PID_FILE_NAME  ${DAEMON_NAME}.pid)
set(DAEMON_LOG_FILE_NAME  ${DAEMON_NAME}.log)
set(DAEMON_NO_CHDIR       1)
set(DAEMON_NO_CLOSE_STDIO 0)


project(${DAEMON_NAME} LANGUAGES C)


set(CMAKE_C_STANDARD          99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS        ON)

set(CMAKE_C_FLAGS_DEBUG   "-DDEBUG -g")
set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG")
set(CMAKE_C_FLAGS         "-O2")


add_compile_options(
    -Wall
    -Wextra
)


if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    message(STATUS "No CMAKE_BUILD_TYPE specified, force set to: ${CMAKE_BUILD_TYPE}")
endif()



list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")


if(USE_SYSTEM_GSOAP)
    #set(GSOAP_DIR "${CMAKE_BINARY_DIR}/gsoap-2.8/gsoap")
    find_package(gSOAP REQUIRED)
else()
    set(GSOAP_VERSION     2.8.66)
    set(GSOAP_INSTALL_DIR "${CMAKE_SOURCE_DIR}")
    set(GSOAP_ARCHIVE_DIR "${CMAKE_SOURCE_DIR}/SDK")
    set(GSOAP_PATCHES     soapcpp2_lex.l.patch)
    set(GSOAP_CONFIGURE   --disable-c-locale --disable-ssl)
    include(build_gsoap)
endif()



add_compile_definitions(
    DAEMON_NAME="${DAEMON_NAME}"
    DAEMON_PID_FILE_NAME="${DAEMON_PID_FILE_NAME}"
    DAEMON_LOG_FILE_NAME="${DAEMON_LOG_FILE_NAME}"
    DAEMON_NO_CHDIR=${DAEMON_NO_CHDIR}
    DAEMON_NO_CLOSE_STDIO=${DAEMON_NO_CLOSE_STDIO}
)



set(COMMON_DIR    "${CMAKE_SOURCE_DIR}/src")
set(WSDL_DIR      "${CMAKE_SOURCE_DIR}/wsdl")
set(GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")



include_directories(
    ${COMMON_DIR}
    ${GENERATED_DIR}
    ${GSOAP_INCLUDE_DIR}
    ${GSOAP_PLUGIN_DIR}
    ${GSOAP_IMPORT_DIR}
)



set(SOAP_SOURCES
    ${GSOAP_PLUGIN_DIR}/wsaapi.c
    ${GSOAP_PLUGIN_DIR}/wsddapi.c
    ${GENERATED_DIR}/soapClient.c
    ${GENERATED_DIR}/soapC.c
)

if(NOT USE_SYSTEM_GSOAP AND NOT USE_GSOAP_STATIC_LIB)
    set(SOAP_SOURCES ${SOAP_SOURCES} ${GSOAP_DIR}/stdsoap2.c)
endif()



set(SOURCES
    ${COMMON_DIR}/${DAEMON_NAME}.c
    ${COMMON_DIR}/daemon.c
    ${COMMON_DIR}/net_utils.c
    ${COMMON_DIR}/file_utils.c
    ${COMMON_DIR}/wsdd_utils.c
    ${COMMON_DIR}/client_events.c

    ${SOAP_SOURCES}
)



set(SOAP_HEADERS
    ${GSOAP_INCLUDE_DIR}/stdsoap2.h
    ${GSOAP_PLUGIN_DIR}/wsaapi.h
    ${GSOAP_PLUGIN_DIR}/wsddapi.h
    ${GENERATED_DIR}/soapH.h
    ${GENERATED_DIR}/soapStub.h
    ${GENERATED_DIR}/wsdd.h
)



set(HEADERS
    ${COMMON_DIR}/smacros.h
    ${COMMON_DIR}/daemon.h
    ${COMMON_DIR}/net_utils.h
    ${COMMON_DIR}/file_utils.h
    ${COMMON_DIR}/wsdd_param.h

    ${GENERATED_DIR}/version.h

    ${SOAP_HEADERS}
)


add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})



#Create a cmake target that generate gsoap files
add_custom_command(
    OUTPUT ${GENERATED_DIR}/wsdd.h
    COMMAND "${CMAKE_COMMAND}" -E make_directory ${GENERATED_DIR}
    COMMAND ${GSOAP_WSDL2H} -cg -t ${GSOAP_WS_DIR}/typemap.dat
            -o ${GENERATED_DIR}/wsdd.h  ${WSDL_DIR}/remotediscovery.wsdl
)


add_custom_command(
    OUTPUT ${GENERATED_DIR}/soapC.c
    COMMAND ${GSOAP_SOAPCPP2} -C -L -x -c -2 -d ${GENERATED_DIR}
            -I${GSOAP_INCLUDE_DIR}:${GSOAP_IMPORT_DIR} ${GENERATED_DIR}/wsdd.h
    DEPENDS ${GENERATED_DIR}/wsdd.h
)


add_custom_target(gsoap_generation
    DEPENDS ${GENERATED_DIR}/soapC.c
)


set_source_files_properties(
    ${GENERATED_DIR}/soapClient.c
    ${GENERATED_DIR}/soapC.c
    ${GENERATED_DIR}/soapH.h
    ${GENERATED_DIR}/soapStub.h
    ${GENERATED_DIR}/wsdd.h
    ${GENERATED_DIR}/version.h
    PROPERTIES GENERATED TRUE
)



add_custom_target(generate_version ALL)

add_custom_command(
    TARGET generate_version
    COMMAND ${CMAKE_COMMAND}
            -DWORK_DIR="${CMAKE_SOURCE_DIR}"
            -DIN_FILE="${CMAKE_SOURCE_DIR}/cmake/version.h.in"
            -DOUT_FILE="${GENERATED_DIR}/version.h"
            -DDAEMON_MAJOR_VERSION=${DAEMON_MAJOR_VERSION}
            -DDAEMON_MINOR_VERSION=${DAEMON_MINOR_VERSION}
            -DDAEMON_PATCH_VERSION=${DAEMON_PATCH_VERSION}
            -P "${CMAKE_SOURCE_DIR}/cmake/version.cmake"
    DEPENDS ${SOURCES}
)

add_dependencies(${PROJECT_NAME} generate_version)


if(USE_SYSTEM_GSOAP OR USE_GSOAP_STATIC_LIB)
    target_link_libraries(${PROJECT_NAME} ${GSOAP_C_LIBRARY})
endif()
