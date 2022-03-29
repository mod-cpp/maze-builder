add_executable(maze-builder cartesian_product.hpp
               compiletime_random.hpp
               main.cpp
               map.hpp
               )

find_package(fmt CONFIG REQUIRED)
target_link_libraries(maze-builder PUBLIC fmt::fmt)
target_compile_options(maze-builder PUBLIC -fconstexpr-ops-limit=999999999)
