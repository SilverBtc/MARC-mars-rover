

project(mars_rover C)
set(CMAKE_C_STANDARD 11)

include_directories(include)

set(SOURCES
    src/main.c
    src/loc.c
    src/moves.c
    src/map.c
    src/queue.c
    src/stack.c
)

add_executable(mars_rover ${SOURCES})


### How to run ###

# Compile (in /build):
# cmake .. && make

# Run:
# ./mars_rover 