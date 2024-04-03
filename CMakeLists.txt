# CMake Build Configuration for root of project
cmake_minimum_required(VERSION 3.18)
project(find_the_dot VERSION 1.0 DESCRIPTION "Assignment 4" LANGUAGES C)

# Compiler options (inherited by sub-folders)
set(CMAKE_C_STANDARD 11)
add_compile_options(-Wall -Werror -Wpedantic -Wextra)
# Disable -Werror=pointer-arith
add_compile_options(-Wno-pointer-arith)
# Enable address sanitizer
# (Comment this out to make your code faster)
add_compile_options(-fsanitize=address)
add_compile_options(-fsanitize=address)

add_link_options(-fsanitize=address)

# What folders to build
add_subdirectory(hal)  
add_subdirectory(app)
add_subdirectory(pru-as4)


 