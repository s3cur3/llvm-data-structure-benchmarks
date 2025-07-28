cmake_minimum_required(VERSION 3.9)
project(llvm_data_structure_benchmarks)

set(CMAKE_CXX_STANDARD 11)


add_executable(llvm_data_structure_benchmarks
		data_structure_benchmarks.cpp
		lib/llvm/support/Error.cpp)
target_include_directories(llvm_data_structure_benchmarks SYSTEM PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/lib/llvm/include")
find_package(benchmark REQUIRED)
target_link_libraries(llvm_data_structure_benchmarks benchmark::benchmark)

target_compile_options(llvm_data_structure_benchmarks PRIVATE
		-march=haswell) # May want to try with -march=native as well
