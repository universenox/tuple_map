cmake_minimum_required(VERSION 3.28)
project(stuff)

# set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_BUILD_TYPE "Release")

# for sanitizers
# set(LLVM_ENABLE_RUNTIMES "compiler-rt")
# set(LLVM_ENABLE_PROJECTS "clang")

# add_compile_options(-std=c++23 -Wall -Wpedantic -Wextra -ggdb3 -O2 -fexperimental-library -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer)
set(CMAKE_CXX_STANDARD 23)
add_compile_options(-Wall -O3 -DNDEBUG -march=native)
# add_compile_options(-std=c++23 -Wall -O3 -fexperimental-library -DNDEBUG -march=native -lstdcxx)

# add_compile_options(-Wall -Wpedantic -Wextra) 

# thread, memory, address are mutually exclusive
# -fsanitize=thread
# -fsanitize=memory
# -fsanitize=kcfi 
# -fsanitize=safe-stack
# -fvisibility=default
# -flto
# -fsanitize=cfi
#-fsanitize=dataflow
#set(sanitizer_args -fsanitize=address -fsanitize=undefined)
# kcfi = kernel indirect call fwd edge control flow integrity

find_package(Boost REQUIRED)
find_package(fmt REQUIRED)
find_package(ut REQUIRED)

add_executable(test test_main.cpp tuple_map.cpp mpl.cpp)
target_compile_definitions(test PUBLIC TEST)
target_link_libraries(test ${BOOST_LIBRARIES} Boost::ut fmt::fmt)


find_package(absl REQUIRED)
find_package(benchmark REQUIRED)
add_executable(bench bench.cpp)
target_link_libraries(bench benchmark::benchmark absl::flat_hash_map fmt::fmt ${BOOST_LIBRARIES})
