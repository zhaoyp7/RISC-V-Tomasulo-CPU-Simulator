#pragma once

#include "branch_predictor.hpp"
#include "memory.hpp"
#include <cstdint>

class Fetch {
private:
  uint32_t pc, last_pc;
  Memory &mem;
  BranchPredictor &bp;

public:
  Fetch(Memory &mem, BranchPredictor &bp)
      : mem(mem), bp(bp), pc(0), last_pc(0) {}
  void fetch(uint32_t &inst, uint32_t &pred_pc) {
    last_pc = pc;
    inst = mem.load_word(pc);
    pred_pc = pc + 4;
    pc = pred_pc;
  }
  void recover_pc(uint32_t actual_pc) { pc = actual_pc; }
  uint32_t get_pc() { return last_pc; }
};