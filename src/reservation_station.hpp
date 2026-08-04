#pragma once

#include "alu.hpp"
#include "decoder.hpp"
#include <cstdint>
#include <cstdio>

const int RS_SIZE = 8;

struct RSData {
  bool busy;
  DecodedIns inst;
  uint32_t Vj, Vk;
  uint8_t Qj, Qk;
  uint8_t tag;
  uint32_t pc;
  int wait_cycles;
  bool is_branch;
  void debug() {
    if (!busy) {puts("idle");return ;}
    printf("%d %u %u %u %u %u %u %d %d\n",busy,Vj,Vk,Qj,Qk,tag,pc,wait_cycles,is_branch);
  }
};

class ReservationStation {
private:
  RSData old_data[RS_SIZE];
  RSData new_data[RS_SIZE];

public:
  ReservationStation() {
    for (int i = 0; i < RS_SIZE; i++) {
      old_data[i].busy = new_data[i].busy = false;
    }
  }
  int insert(const DecodedIns &inst, uint8_t tag, uint32_t Vj, uint32_t Vk,
             uint8_t Qj, uint8_t Qk, uint32_t pc) {
    for (int i = 0; i < RS_SIZE; i++) {
      if (old_data[i].busy == false) {
        new_data[i] = (RSData){true, inst, Vj, Vk, Qj,
                               Qk,   tag,  pc, 1,  (inst.opcode == 0x63)};
        return i;
      }
    }
    return -1;
  }
  bool check_full() {
    for (int i = 0; i < RS_SIZE; i++) {
      if (old_data[i].busy == false) {
        return false;
      }
    }
    return true;
  }
  int find_ready() {
    for (int i = 0; i < RS_SIZE; i++) {
      if (old_data[i].busy && old_data[i].Qj == 0 && old_data[i].Qk == 0) {
        return i;
      }
    }
    return -1;
  }
  void execute() {
    for (int i = 0; i < RS_SIZE; i++) {
      if (old_data[i].busy && old_data[i].Qj == 0 && old_data[i].Qk == 0 &&
          old_data[i].wait_cycles) {
        new_data[i].wait_cycles--;
      }
    }
  }
  ALUResult get_result(int idx) {
    ALUResult alu = ALU::execute(old_data[idx].inst, old_data[idx].Vj,
                                 old_data[idx].Vk, old_data[idx].pc);
    return alu;
  }
  uint8_t get_tag(int idx) { return old_data[idx].tag; }
  uint32_t get_pc(int idx) { return old_data[idx].pc; }
  void remove(int idx) { new_data[idx].busy = false; }
  bool check_ready(int idx) {
    return (old_data[idx].busy && !old_data[idx].Qj && !old_data[idx].Qk);
  }
  bool check_done(int idx) {
    return (check_ready(idx) && !old_data[idx].wait_cycles);
  }
  void flush() {
    for (int i = 0; i < RS_SIZE; i++) {
      new_data[i].busy = false;
    }
  }
  void listen_cdb(uint8_t tag, uint32_t value) {
    for (int i = 0; i < RS_SIZE; i++) {
      if (old_data[i].busy) {
        if (old_data[i].Qj != 0 && old_data[i].Qj == tag) {
          new_data[i].Vj = value;
          new_data[i].Qj = 0;
        }
        if (old_data[i].Qk != 0 && old_data[i].Qk == tag) {
          new_data[i].Vk = value;
          new_data[i].Qk = 0;
        }
      }
    }
  }
  void tick() {
    for (int i = 0; i < RS_SIZE; i++) {
      old_data[i] = new_data[i];
    }
  }
  void debug() {
    puts("RS debug");
    puts("OLD DEBUG");
    for (int i = 0; i < RS_SIZE; i++) old_data[i].debug();
    puts("NEW DEBUG");
    for (int i = 0; i < RS_SIZE; i++) new_data[i].debug();
  }
};