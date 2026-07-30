#include "naive-src/decoder.hpp"
#include "naive-src/executer.hpp"
#include "naive-src/memory.hpp"
#include "naive-src/regfile.hpp"

#include <cstdio>

int main() {
  RegFile reg;
  Decoder decoder;
  Executor executor;
  Memory mem;
  mem.init();
  uint32_t pc = 0;
  while (true) {
    uint32_t ins = mem.load_word(pc);
    if (ins == 0x0ff00513) {
      printf("%u", reg.read(10) & 0xFF);
      return 0;
    }
    DecodedIns decoded = decoder.decode(ins);
    ExecuteResult result = executor.execute(decoded, reg, mem, pc);
    reg.tick();
    pc = result.next_pc;
  }
  return 0;
}