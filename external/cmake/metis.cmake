#
#  Copyright 2009-2015, Jack Poulson
#  All rights reserved.
#
#  This file is part of Elemental and is under the BSD 2-Clause License,
#  which can be found in the LICENSE file in the root directory, or at
#  http://opensource.org/licenses/BSD-2-Clause
#
include(ExternalProject)

if(NOT BUILD_METIS)
  find_package(METIS)
endif()

if(METIS_FOUND)
  # find_package returns 'METIS_LIBRARIES' but METIS's CMakeLists.txt
  # returns 'METIS_LIBS'
  set(METIS_LIBS ${METIS_LIBRARIES})
  include_directories(${METIS_INCLUDE_DIRS}) 
else()
  if(NOT DEFINED METIS_URL)
    set(METIS_URL https://github.com/poulson/Metis.git)
  endif()
  message(STATUS "Will pull METIS from ${METIS_URL}")

  set(METIS_SOURCE_DIR ${PROJECT_SOURCE_DIR}/external/metis)
  set(METIS_BINARY_DIR ${PROJECT_BINARY_DIR}/external/metis)

  ExternalProject_Add(project_metis 
    PREFIX ${CMAKE_INSTALL_PREFIX}
    GIT_REPOSITORY ${METIS_URL}
    STAMP_DIR  ${METIS_BINARY_DIR}/stamp
    SOURCE_DIR ${METIS_SOURCE_DIR}
    BINARY_DIR ${METIS_BINARY_DIR}
    TMP_DIR    ${METIS_BINARY_DIR}/tmp
    CMAKE_ARGS -D CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
               -D BUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
               -D METIS_INSTALL=ON
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    INSTALL_COMMAND ""
    UPDATE_COMMAND "" 
  )
  add_dependencies(External project_metis)

  ExternalProject_Get_Property(project_metis install_dir)
  if(BUILD_SHARED_LIBS)
    add_library(libmetis SHARED IMPORTED)
    set_property(TARGET libmetis PROPERTY IMPORTED_LOCATION ${install_dir}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}metis${CMAKE_SHARED_LIBRARY_SUFFIX})
  else()
    add_library(libmetis STATIC IMPORTED)
    set_property(TARGET libmetis PROPERTY IMPORTED_LOCATION ${install_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}metis${CMAKE_STATIC_LIBRARY_SUFFIX})
  endif() 
 
  set(METIS_LIBS libmetis)
  set(EL_BUILT_METIS TRUE)
endif()

set(EXTERNAL_LIBS ${EXTERNAL_LIBS} ${METIS_LIBS})

set(EL_HAVE_METIS TRUE)