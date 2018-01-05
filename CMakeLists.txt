# CMakeLists.txt pour passe_temps
# vim:set ts=2 sw=2 et:

cmake_minimum_required(VERSION 3.1)

project(passe_temps)

find_package(Boost REQUIRED COMPONENTS chrono system)

include_directories(${Boost_INCLUDE_DIRS})
add_executable(passe_temps passe_temps.cpp)

target_link_libraries(passe_temps ${Boost_LIBRARIES})
target_compile_features(passe_temps PRIVATE cxx_range_for)


enable_testing()
add_test(A passe_temps)
