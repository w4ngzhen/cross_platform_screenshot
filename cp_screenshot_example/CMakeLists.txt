cmake_minimum_required(VERSION 3.26)

project(cp_screenshot_example)

if (APPLE)
  # set macOS link flags about framework.
  set(CMAKE_EXE_LINKER_FLAGS "-framework Cocoa -framework AppKit -framework CoreData -framework Foundation -framework ImageIO")
endif ()

# ======================================== #
#                  Example                 #
# ======================================== #

add_executable(
    cp_screenshot_example
    main.cpp
)

add_dependencies(
    cp_screenshot_example
    cp_screenshot_core
)

target_include_directories(
    cp_screenshot_example
    PRIVATE
    ./
    ../cp_screenshot_core/include
)

target_link_libraries(
    cp_screenshot_example
    PRIVATE
    cp_screenshot_core
)