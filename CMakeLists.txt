cmake_minimum_required(VERSION 3.16.3)

project(chimes)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")

include_directories(include local/include)
link_directories(local/lib)

add_compile_options(-Wall -fPIC)
set(LIBS rtaudio m dl pthread)

add_executable(chimes src/chimes.c src/main.c)
target_link_libraries(chimes ${LIBS})


