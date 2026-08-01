#include "src/tomasulo.hpp"

int main() {
  Tomasulo cpu;
  cpu.init();
  while (!cpu.check_done()) {
    cpu.step();
  }
  // printf("total cycles = %d\n",cpu.get_cycles());
  // cpu.bp_result();
  printf("%u", cpu.get_result());
  return 0;
}