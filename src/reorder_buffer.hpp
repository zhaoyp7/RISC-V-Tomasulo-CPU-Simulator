#pragma once

#include "decoder.hpp"
#include <cstdint>

const int ROB_SIZE = 8;

enum class ROBState {ISSUE, EXECUTE, WRITE, COMMIT};

struct ROBData{
  bool busy;
  ROBState state;
  DecodedIns inst;
  uint32_t value;
  uint8_t dest;
  bool is_branch;
  bool go_branch;
  uint32_t pc;
  uint32_t pred_pc;
  uint32_t actural_pc;
  int lsb_idx;
};

class ReorderBuffer {
private:
  ROBData old_data[ROB_SIZE];
  ROBData new_data[ROB_SIZE];
  int old_head, old_tail;
  int new_head, new_tail;

public:
  ReorderBuffer();
  int insert(const DecodedIns &inst, uint32_t pc, uint32_t  pred_pc);
  bool check_full();
  void set_execute(int idx);
  void set_write(int idx);
  void set_lsb_idx(int idx, int lsb_idx);
  bool check_commit();
  ROBData& commit();
  void pop();
  void flush(int idx);
  bool is_branch(int idx);
  bool is_store(int idx);
  uint32_t get_actual_pc(int idx);
  int get_lsb_idx(int idx) const;
  void tick();
};