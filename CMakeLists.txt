cmake_minimum_required(VERSION 3.28.2)

include(../../sdk/config.cmake)
project(sieelf_sie_pd C)

include_directories("../libpd/src")
link_libraries(crt crt_helper gcc c pd)
file(GLOB_RECURSE SOURCES "src/*.c")

add_definitions(-DSWILIB_MODERN)

add_executable(SiePD_ELKA ${SOURCES})
target_sdk_setup(SiePD_ELKA ELKA)
target_compile_options(SiePD_ELKA BEFORE PUBLIC -std=c11 -Wall -Werror)
