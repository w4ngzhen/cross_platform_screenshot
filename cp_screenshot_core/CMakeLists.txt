cmake_minimum_required(VERSION 3.26)

project(cp_screenshot_core C CXX OBJC OBJCXX)

set(CMAKE_CXX_STANDARD 11)

# ======================================== #
#     Platform-specified configuration     #
# ======================================== #
if (WIN32)
  message(STATUS "Compile Windows lib.")
  set(SOURCE_FILES "src/screenshot_os_win.cpp")
elseif (APPLE)
  message(STATUS "Compile macOS lib.")
  set(SOURCE_FILES "src/screenshot_os_mac.mm")
  # set macOS link flags about framework.
  set(CMAKE_EXE_LINKER_FLAGS "-framework Cocoa")
elseif (UNIX)
  message(STATUS "Compile unix/linux X11 lib.")
  set(SOURCE_FILES "src/screenshot_os_linux.cpp")
else ()
  message(FATAL_ERROR "Unsupported platform.")
endif ()

# library target
add_library(
    cp_screenshot_core
    ${SOURCE_FILES}
)

target_include_directories(cp_screenshot_core PUBLIC include)