cmake_minimum_required(VERSION 3.15)

project(graphics-sandbox LANGUAGES CXX)

# ---- Dependencies ----
include(cmake/CPM.cmake)
include(Dependencies.cmake)

# ---- Application ----

set(APP_TARGET graphics-sandbox)

file(GLOB_RECURSE APP_HEADERS src/**.hpp)
file(GLOB_RECURSE APP_SOURCES src/**.cpp)
add_executable(${APP_TARGET} ${APP_HEADERS} ${APP_SOURCES})
target_include_directories(${APP_TARGET} PRIVATE src)
set_target_properties(${APP_TARGET}
        PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED On
        CXX_EXTENSIONS Off
)

target_link_libraries(${APP_TARGET} PRIVATE fmt glm glfw VkMana assimp stb)