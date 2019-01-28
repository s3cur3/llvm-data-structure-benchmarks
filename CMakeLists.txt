cmake_minimum_required(VERSION 3.9)
project(llvm_data_structure_benchmarks)

set(CMAKE_CXX_STANDARD 11)


add_executable(llvm_data_structure_benchmarks
		data_structure_benchmarks.cpp
		lib/llvm/support/Error.cpp)
target_include_directories(llvm_data_structure_benchmarks SYSTEM PRIVATE
		"/usr/local/Cellar/google-benchmark/1.3.0/include/" # Get this on Mac from $ brew install google-benchmark
		"${CMAKE_CURRENT_SOURCE_DIR}/lib/llvm/include")
find_library(GOOGLE_BENCHMARK benchmark)
target_link_libraries(llvm_data_structure_benchmarks
		${GOOGLE_BENCHMARK})

target_compile_options(llvm_data_structure_benchmarks PRIVATE
		-march=haswell) # May want to try with -march=native as well
