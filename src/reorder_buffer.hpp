#pragma once

#include "decoder.hpp"
#include <cstdint>

const int ROB_SIZE = 8;

enum class ROBState { ISSUE, EXECUTE, WRITE, COMMIT };

struct ROBData {
  bool busy;
  ROBState state;
  DecodedIns inst;
  uint32_t value;
  uint8_t dest;
  bool is_branch;
  bool go_branch;
  uint32_t pc;
  uint32_t pred_pc;
  uint32_t actual_pc;
  int lsq_idx;
};

class ReorderBuffer {
private:
  ROBData old_data[ROB_SIZE];
  ROBData new_data[ROB_SIZE];
  int old_head, old_tail;
  int new_head, new_tail;

public:
  ReorderBuffer() {
    for (int i = 0; i < ROB_SIZE; i++) {
      old_data[i].busy = new_data[i].busy = false;
    }
    old_head = old_tail = 0;
    new_head = new_tail = 0;
  }
  int insert(const DecodedIns &inst, uint32_t pc, uint32_t pred_pc) {
    int idx = new_tail;
    if (new_data[idx].busy) {
      return -1;
    }
    new_tail = (new_tail + 1) % ROB_SIZE;
    bool tmp =
        (inst.opcode == 0x63 || inst.opcode == 0x67 || inst.opcode == 0x6F);
    new_data[idx] =
        (ROBData){true, ROBState::ISSUE, inst,   0, inst.rd, tmp, false,
                  pc,   pred_pc,         pc + 4, -1};
    return idx;
  }
  void set_write(int idx, uint32_t value) {
    new_data[idx].state = ROBState::WRITE;
    new_data[idx].value = value;
  }
  void set_branch(int idx, bool go_branch, uint32_t actual_pc) {
    new_data[idx].go_branch = go_branch;
    new_data[idx].actual_pc = actual_pc;
  }
  void set_execute(int idx) { new_data[idx].state = ROBState::EXECUTE; }
  void set_lsq_idx(int idx, int lsq_idx) { new_data[idx].lsq_idx = lsq_idx; }
  bool check_full() { return (old_data[old_tail].busy); }
  bool check_empty() { return (old_data[old_head].busy == false); }
  bool is_branch(int idx) { return old_data[idx].is_branch; }
  bool is_store(int idx) { return (old_data[idx].inst.opcode == 0x23); }
  uint32_t get_actual_pc(int idx) { return old_data[idx].actual_pc; }
  int get_lsq_idx(int idx) { return old_data[idx].lsq_idx; }
  bool check_commit() {
    int idx = new_head;
    if (new_data[idx].busy == false) {
      return false;
    }
    return (new_data[idx].state == ROBState::WRITE);
  }
  ROBData commit() {
    int idx = new_head;
    new_data[idx].state = ROBState::COMMIT;
    new_data[idx].busy = false;
    new_head = (new_head + 1) % ROB_SIZE;
    return old_data[idx];
  }
  void flush(int idx) {
    while (new_tail != idx) {
      new_tail = (new_tail - 1 + ROB_SIZE) % ROB_SIZE;
      new_data[new_tail].busy = false;
    }
    new_head = idx;
  }
  void flush_all() {
    while (new_tail != new_head) {
      new_tail = (new_tail - 1 + ROB_SIZE) % ROB_SIZE;
      new_data[new_tail].busy = false;
    }
  }
  void tick() {
    for (int i = 0; i < ROB_SIZE; i++) {
      old_data[i] = new_data[i];
    }
    old_head = new_head;
    old_tail = new_tail;
  }
};