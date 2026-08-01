cmake_minimum_required(VERSION 3.22.1)
project(RISC-V-Tomasulo-CPU-Simulator)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Wall)
add_compile_options(-Ofast)

add_executable(naive-code 

  naive-src/decoder.hpp
  naive-src/memory.hpp
  naive-src/executer.hpp
  naive-src/regfile.hpp

  naive-main.cpp
)

add_executable(code 
  src/alu.hpp
  src/branch_predictor.hpp
  src/cdb.hpp
  src/decoder.hpp
  src/fetch.hpp
  src/load_store_queue.hpp
  src/memory.hpp
  src/regfile.hpp
  src/reorder_buffer.hpp
  src/reservation_station.hpp
  src/tomasulo.hpp

  main.cpp
)