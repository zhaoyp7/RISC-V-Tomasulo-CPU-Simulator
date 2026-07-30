#pragma once

#include "branch_predictor.hpp"
#include "memory.hpp"
#include <cstdint>

class Fetch {
private:
  uint32_t pc;
  Memory &mem;
  BranchPredictor &bp;

public:
  Fetch();
  bool fetch(uint32_t &inst, uint32_t &pred_pc);
  void recover(uint32_t actural_pc);
  uint32_t get_pc();
};