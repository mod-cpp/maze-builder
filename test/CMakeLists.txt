enable_testing()

find_package(Catch2 REQUIRED)
find_package(fmt CONFIG REQUIRED)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "*.cpp")
add_executable(test_maze_builder ${sources})
target_include_directories(test_maze_builder PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(test_maze_builder PRIVATE fmt::fmt Catch2::Catch2WithMain)
target_compile_options(test_maze_builder PUBLIC -fconstexpr-ops-limit=9999999999)

include(CTest)
include(Catch)
catch_discover_tests(test_maze_builder)
