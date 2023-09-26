cmake_minimum_required(VERSION 3.26)

project(cross_platform_screenshot)

set(CMAKE_CXX_STANDARD 11)

# ======================================== #
#     Platform-specified configuration     #
# ======================================== #
set(SCREENSHOT_HEADER "src/screenshot.h")
if (WIN32)
  message(STATUS "Compile Windows lib.")
  set(SCREENSHOT_SRC_FILES "src/screenshot_os_win.cpp")
elseif (APPLE)
  message(STATUS "Compile macOS lib.")
  set(SCREENSHOT_SRC_FILES "src/screenshot_os_mac.mm")
  # enable objective-c and -cxx compile.
  set(CMAKE_OBJCXX_COMPILE_OBJECT ON)
  # set macOS link flags about framework.
  set(CMAKE_EXE_LINKER_FLAGS "-framework Cocoa -framework AppKit -framework CoreData -framework Foundation -framework ImageIO")
elseif (UNIX)
  message(STATUS "Compile unix/linux X11 lib.")
  set(SCREENSHOT_SRC_FILES "src/screenshot_os_linux.cpp")
else ()
  message(FATAL_ERROR "Unsupported platform.")
endif ()

# ======================================== #
#                  Example                 #
# ======================================== #
add_executable(screenshot_example
    src/an_example.cpp
    ${SCREENSHOT_HEADER}
    ${SCREENSHOT_SRC_FILES})