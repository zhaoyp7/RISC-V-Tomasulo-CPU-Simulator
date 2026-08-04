#pragma once

#include "decoder.hpp"
#include <cstdint>
#include <cstdio>

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
  void debug() {
    if (!busy) {puts("idle");return ;}
    printf("%d %d %u %u %d %d %u %u %u %d\n",busy,(int)state,value,dest,is_branch,go_branch,pc,pred_pc,actual_pc,lsq_idx);
  }
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
    int idx = old_tail;
    if (old_data[idx].busy) {
      return -1;
    }
    new_tail = (old_tail + 1) % ROB_SIZE;
    bool tmp =
        (inst.opcode == 0x63 || inst.opcode == 0x67 || inst.opcode == 0x6F);
    new_data[idx] =
        (ROBData){true, ROBState::ISSUE, inst,   0, inst.rd, tmp, false,
                  pc,   pred_pc,         pc + 4, -1};
    return idx + 1;
  }
  void set_write(int tag, uint32_t value) {
    int idx = tag - 1;
    new_data[idx].state = ROBState::WRITE;
    new_data[idx].value = value;
  }
  void set_branch(int tag, bool go_branch, uint32_t actual_pc) {
    int idx = tag - 1;
    new_data[idx].go_branch = go_branch;
    new_data[idx].actual_pc = actual_pc;
  }
  void set_execute(int tag) {
    int idx = tag - 1;
    new_data[idx].state = ROBState::EXECUTE;
  }
  void set_lsq_idx(int tag, int lsq_idx) {
    int idx = tag - 1;
    new_data[idx].lsq_idx = lsq_idx;
  }
  bool check_full() { return (old_data[old_tail].busy); }
  bool check_empty() { return (old_data[old_head].busy == false); }
  bool is_branch(int tag) {
    int idx = tag - 1;
    return old_data[idx].is_branch;
  }
  bool is_store(int tag) {
    int idx = tag - 1;
    return (old_data[idx].inst.opcode == 0x23);
  }
  uint32_t get_actual_pc(int tag) {
    int idx = tag - 1;
    return old_data[idx].actual_pc;
  }
  int get_lsq_idx(int tag) {
    int idx = tag - 1;
    return old_data[idx].lsq_idx;
  }
  bool check_commit() {
    int idx = old_head;
    if (old_data[idx].busy == false) {
      return false;
    }
    return (old_data[idx].state == ROBState::WRITE);
  }
  ROBData commit() {
    int idx = old_head;
    new_data[idx].state = ROBState::COMMIT;
    new_data[idx].busy = false;
    new_head = (old_head + 1) % ROB_SIZE;
    return old_data[idx];
  }
  void flush() {
    for (int i = 0; i < ROB_SIZE; i++) {
      new_data[i].busy = false;
    }
    new_head = new_tail = 0;
  }
  void tick() {
    for (int i = 0; i < ROB_SIZE; i++) {
      old_data[i] = new_data[i];
    }
    old_head = new_head;
    old_tail = new_tail;
  }
  void debug() {
    puts("ROB debug");
    printf("old_head = %d, old_tail = %d\n",old_head,old_tail);
    for (int i = 0; i < ROB_SIZE; i++) old_data[i].debug();
    printf("new_head = %d, new_tail = %d\n",new_head,new_tail);
    for (int i = 0; i < ROB_SIZE; i++) new_data[i].debug();
  }
};