#***************************************************************************
#                                  _   _ ____  _
#  Project                     ___| | | |  _ \| |
#                             / __| | | | |_) | |
#                            | (__| |_| |  _ <| |___
#                             \___|\___/|_| \_\_____|
#
# Copyright (C) 1998 - 2020, Daniel Stenberg, <daniel@haxx.se>, et al.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at https://curl.haxx.se/docs/copyright.html.
#
# You may opt to use, copy, modify, merge, publish, distribute and/or sell
# copies of the Software, and permit persons to whom the Software is
# furnished to do so, under the terms of the COPYING file.
#
# This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
# KIND, either express or implied.
#
###########################################################################
set(EXE_NAME curl)
set(LIB_NAME libcurltool)
IF(UWP)
  ADD_DEFINITIONS(-DUWPBUILD)
ENDIF(UWP)

if(USE_MANUAL)
  # Use the C locale to ensure that only ASCII characters appear in the
  # embedded text. NROFF and MANOPT are set in the parent CMakeLists.txt
  add_custom_command(
    OUTPUT tool_hugehelp.c
    COMMAND ${CMAKE_COMMAND} -E echo "#include \"tool_setup.h\"" > tool_hugehelp.c
    COMMAND ${CMAKE_COMMAND} -E echo "#ifndef HAVE_LIBZ" >> tool_hugehelp.c
    COMMAND env LC_ALL=C "${NROFF}" ${NROFF_MANOPT}
            "${CURL_BINARY_DIR}/docs/curl.1" |
            "${PERL_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/mkhelp.pl" >> tool_hugehelp.c
    COMMAND ${CMAKE_COMMAND} -E echo "#else" >> tool_hugehelp.c
    COMMAND env LC_ALL=C "${NROFF}" ${NROFF_MANOPT}
            "${CURL_BINARY_DIR}/docs/curl.1" |
            "${PERL_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/mkhelp.pl" -c >> tool_hugehelp.c
    COMMAND ${CMAKE_COMMAND} -E echo "#endif /* HAVE_LIBZ */" >> tool_hugehelp.c
    DEPENDS
      generate-curl.1
      "${CURL_BINARY_DIR}/docs/curl.1"
      "${CMAKE_CURRENT_SOURCE_DIR}/mkhelp.pl"
      "${CMAKE_CURRENT_SOURCE_DIR}/tool_hugehelp.h"
    VERBATIM)
else()
  add_custom_command(
    OUTPUT tool_hugehelp.c
    COMMAND ${CMAKE_COMMAND} -E echo "/* built-in manual is disabled, blank function */" > tool_hugehelp.c
    COMMAND ${CMAKE_COMMAND} -E echo "#include \"tool_hugehelp.h\"" >> tool_hugehelp.c
    COMMAND ${CMAKE_COMMAND} -E echo "void hugehelp(void) {}" >> tool_hugehelp.c
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/tool_hugehelp.h"
    VERBATIM)

endif()

transform_makefile_inc("Makefile.inc" "${CMAKE_CURRENT_BINARY_DIR}/Makefile.inc.cmake")
include(${CMAKE_CURRENT_BINARY_DIR}/Makefile.inc.cmake)

if(MSVC)
  list(APPEND CURL_FILES curl.rc)
endif()

add_library(
  ${LIB_NAME} SHARED
  ${CURL_FILES}
  )

set_target_properties(${LIB_NAME} PROPERTIES COMPILE_DEFINITIONS "CUSTOM_API_EXPORT")
set_target_properties(${LIB_NAME} PROPERTIES PREFIX "")
set_target_properties(${LIB_NAME} PROPERTIES IMPORT_PREFIX "")

if(WIN32)
  if(BUILD_SHARED_LIBS)
    # Add "_imp" as a suffix before the extension to avoid conflicting with the statically linked "libcurl.lib"
    set_target_properties(${LIB_NAME} PROPERTIES IMPORT_SUFFIX "_imp.lib")
  endif()
endif()

target_link_libraries( ${LIB_NAME} libcurl ${CURL_LIBS})

# CURL_FILES comes from Makefile.inc
add_executable(
  ${EXE_NAME}
  ${CURL_FILES}
  )
add_dependencies(${EXE_NAME} ${LIB_NAME})

add_executable(
  ${PROJECT_NAME}::${EXE_NAME}
  ALIAS ${EXE_NAME}
  )

if(CURL_HAS_LTO)
  set_target_properties(${EXE_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
    INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)
endif()

if(ENABLE_UNICODE AND MINGW)
  target_link_libraries(${EXE_NAME} -municode)
  # GCC doesn't know about wmain
  set_source_files_properties(tool_main.c PROPERTIES COMPILE_FLAGS "-Wno-missing-prototypes -Wno-missing-declarations")
endif()

source_group("curlX source files" FILES ${CURLX_CFILES})
source_group("curl source files" FILES ${CURL_CFILES})
source_group("curl header files" FILES ${CURL_HFILES})

include_directories(
  ${CURL_SOURCE_DIR}/lib        # To be able to reach "curl_setup_once.h"
  ${CURL_BINARY_DIR}/lib        # To be able to reach "curl_config.h"
  ${CURL_BINARY_DIR}/include    # To be able to reach "curl/curl.h"
  # This is needed as tool_hugehelp.c is generated in the binary dir
  ${CURL_SOURCE_DIR}/src        # To be able to reach "tool_hugehelp.h"
  )

#Build curl executable
target_link_libraries(${EXE_NAME} libcurl ${CURL_LIBS})
#IF(USE_LIBRTMP)
#  target_link_libraries( ${EXE_NAME} "-lrtmp")
#ENDIF(USE_LIBRTMP)

# TEST WRAPPER
add_executable(
  testwrapper
  testwrapper.c
)

target_link_libraries( testwrapper ${LIB_NAME} ${CURL_LIBS})
IF(NOT WIN32)
  target_link_libraries( testwrapper "-pthread")
ENDIF(NOT WIN32)
#IF(USE_LIBRTMP)
#  target_link_libraries( testwrapper "-lrtmp")
#ENDIF(USE_LIBRTMP)


#target_link_libraries( testWrapperDummy ${DUMMY_LIB_NAME})

################################################################################

#SET_TARGET_PROPERTIES(${EXE_NAME} ARCHIVE_OUTPUT_DIRECTORY "blah blah blah")
#SET_TARGET_PROPERTIES(${EXE_NAME} RUNTIME_OUTPUT_DIRECTORY "blah blah blah")
#SET_TARGET_PROPERTIES(${EXE_NAME} LIBRARY_OUTPUT_DIRECTORY "blah blah blah")

# Add the postfix to the executable since it is not added automatically as for modules and shared libraries
set_target_properties(${EXE_NAME} PROPERTIES
  DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")

set_target_properties(${LIB_NAME} PROPERTIES
  DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")

#INCLUDE(ModuleInstall OPTIONAL)

install(TARGETS ${LIB_NAME} EXPORT ${TARGETS_EXPORT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS ${EXE_NAME} EXPORT ${TARGETS_EXPORT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
export(TARGETS ${EXE_NAME}
       APPEND FILE ${PROJECT_BINARY_DIR}/curl-target.cmake
       NAMESPACE ${PROJECT_NAME}::
)
