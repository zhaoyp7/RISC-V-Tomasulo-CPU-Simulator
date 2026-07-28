cmake_minimum_required(VERSION 3.22.1)
project(RISC-V-Tomasulo-CPU-Simulator)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Wall)
add_compile_options(-Ofast)

add_executable(code 

  src/decoder.hpp
  src/memory.hpp
  src/executer.hpp
  src/regfile.hpp

  naive-main.cpp
)