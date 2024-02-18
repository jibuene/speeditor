cmake_minimum_required(VERSION 3.20)
project(speEditor
        VERSION 0.1
        DESCRIPTION "A simple text editor"
        LANGUAGES C)

add_executable(speEditor src/main.c)
target_compile_features(speEditor PRIVATE c_std_23)

# target_compile_options(speEditor PRIVATE -Wall -Wextra -pedantic -Werror)
find_package(raylib QUIET)
target_link_libraries(${PROJECT_NAME} raylib)
target_link_libraries(${PROJECT_NAME} m)
include_directories(ThirdParty)
