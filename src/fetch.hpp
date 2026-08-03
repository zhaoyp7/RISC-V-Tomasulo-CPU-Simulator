#pragma once

#include "branch_predictor.hpp"
#include "memory.hpp"
#include <cstdint>

class Fetch {
private:
  uint32_t pc, last_pc;
  Memory &mem;
  BranchPredictor &bp;
  uint32_t decode_J_imm(uint32_t inst) {
    uint32_t imm = (int32_t(inst) >> 31) << 20;
    imm |= ((inst >> 12) & 0xFF) << 12;
    imm |= ((inst >> 20) & 0x1) << 11;
    imm |= ((inst >> 21) & 0x3FF) << 1;
    return imm;
  }
  uint32_t decode_B_imm(uint32_t inst) {
    uint32_t imm = (int32_t(inst) >> 31) << 12;
    imm |= ((inst >> 7) & 0x1) << 11;
    imm |= ((inst >> 25) & 0x3F) << 5;
    imm |= ((inst >> 8) & 0xF) << 1;
    return imm;
  }

public:
  Fetch(Memory &mem, BranchPredictor &bp)
      : mem(mem), bp(bp), pc(0), last_pc(0) {}
  void fetch(uint32_t &inst, uint32_t &pred_pc) {
    last_pc = pc;
    inst = mem.load_word(pc);
    uint8_t opcode = inst & 0x7F;
    if (opcode == 0x6F) {
      pred_pc = pc + decode_J_imm(inst);
    } else if (opcode == 0x63 && bp.predict(pc)) {
      pred_pc = pc + decode_B_imm(inst);
    } else {
      pred_pc = pc + 4;
    }
    pc = pred_pc;
  }
  void recover_pc(uint32_t actual_pc) { pc = actual_pc; }
  uint32_t get_pc() { return last_pc; }
};