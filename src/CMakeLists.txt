add_executable(maze-builder main.cpp)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(maze-builder PUBLIC fmt::fmt)
target_compile_options(maze-builder PUBLIC -fconstexpr-ops-limit=9999999999)
