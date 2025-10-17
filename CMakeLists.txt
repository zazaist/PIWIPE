cmake_minimum_required(VERSION 3.15)
project(piwiper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(piwiper
   src/main.cpp
)

add_executable(piwiper-gui WIN32
   src/main_gui.cpp
)

target_link_libraries(piwiper-gui comctl32 user32 gdi32 advapi32)

# Embed manifest for administrator privileges
if (MSVC)
  set_target_properties(piwiper-gui PROPERTIES
    LINK_FLAGS "/MANIFEST:EMBED /MANIFESTINPUT:${CMAKE_SOURCE_DIR}/src/piwiper-gui.manifest"
  )
endif()

if (MSVC)
  target_compile_options(piwiper PRIVATE /W4 /EHsc)
else()
  target_compile_options(piwiper PRIVATE -Wall -Wextra -Wpedantic)
endif()
