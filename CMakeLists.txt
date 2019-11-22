cmake_minimum_required(VERSION 3.10)

project(StackWalkAsAService)

include(GNUInstallDirs)

# Common options
if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
    "Set the build type, usually Release or RelWithDebInfo" FORCE)
endif(CMAKE_BUILD_TYPE STREQUAL "")

find_package(Threads)

# Boost
set(NEEDED_BOOST_COMPONENTS program_options system filesystem)

find_package(Boost COMPONENTS ${NEEDED_BOOST_COMPONENTS})

if(NOT Boost_FOUND)
  # Automatically print stuff if it failed
  set(Boost_DEBUG ON)
  find_package(Boost COMPONENTS ${NEEDED_BOOST_COMPONENTS})
  message(FATAL_ERROR "Failed to find Boost libraries")
endif(NOT Boost_FOUND)

# Boost is found or the configuration has already failed

include_directories(${Boost_INCLUDE_DIR})

# All boost libs are linked by cmake
add_definitions(-DBOOST_ALL_NO_LIB)
add_definitions(-DBOOST_PROGRAM_OPTIONS_DYN_LINK=1)

# RubySetupSystem installed things
link_directories(build/ThirdParty/lib)
include_directories(build/ThirdParty/include)

# Copy stackwalk to the build folder and also install it
set(STACK_WALK_EXECUTABLE build/ThirdParty/bin/minidump_stackwalk)

# Wt libs (static linking is used on windows)
if(UNIX)
  file(GLOB WT_LIBRARIES build/ThirdParty/lib/libwt*.so*)

  install(FILES ${WT_LIBRARIES} DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:${CMAKE_INSTALL_LIBDIR}")

# Core Wt resources
set(WT_RESOURCES_FOLDER build/ThirdParty/share/Wt/resources)
set(WT_CONFIG_FILE config/wt_config.xml)


file(COPY ${STACK_WALK_EXECUTABLE} DESTINATION "${CMAKE_BINARY_DIR}/src")
install(PROGRAMS ${STACK_WALK_EXECUTABLE} DESTINATION ${CMAKE_INSTALL_BINDIR})


file(COPY ${WT_RESOURCES_FOLDER} DESTINATION "${CMAKE_BINARY_DIR}/src")
install(DIRECTORY ${WT_RESOURCES_FOLDER} DESTINATION
  ${CMAKE_INSTALL_LOCALSTATEDIR}/StackWalkAsAService/http)

install(FILES ${WT_CONFIG_FILE} DESTINATION
  ${CMAKE_INSTALL_SYSCONFDIR}/StackWalkAsAService/)




add_subdirectory(src)
# add_subdirectory(test)
