#pragma once

#include "decoder.hpp"
#include <cstdint>

struct ALUResult {
  uint32_t value;
  uint32_t next_pc;
  bool go_branch;
  bool is_branch;
  ALUResult() {
    value = 0;
    next_pc = 0;
    go_branch = 0;
    is_branch = 0;
  }
};

class ALU {
private:
  DecodedIns old_inst, new_inst;
  uint32_t old_rs1, new_rs1;
  uint32_t old_rs2, new_rs2;
  uint32_t old_pc, new_pc;
  uint8_t old_data_tag, new_data_tag;
  uint8_t old_result_tag, new_result_tag;
  ALUResult old_res, new_res;
  bool old_data_ready, new_data_ready;
  bool old_result_ready, new_result_ready;

public:
  ALU() {
    old_inst = new_inst = DecodedIns();
    old_rs1 = new_rs1 = 0;
    old_rs2 = new_rs2 = 0;
    old_pc = new_pc = 0;
  }
  void run() {
    if (!old_data_ready) {
      return;
    }
    DecodedIns inst = old_inst;
    uint32_t rs1 = old_rs1;
    uint32_t rs2 = old_rs2;
    uint32_t pc = old_pc;
    ALUResult res;
    res.value = 0;
    res.next_pc = pc + 4;
    res.is_branch = res.go_branch = 0;
    if (inst.opcode == 0x33) {
      uint32_t ans = 0;
      if (inst.funct3 == 0x0) {
        ans = (inst.funct7 == 0x0) ? (rs1 + rs2) : (rs1 - rs2);
      } else if (inst.funct3 == 0x7) {
        ans = rs1 & rs2;
      } else if (inst.funct3 == 0x6) {
        ans = rs1 | rs2;
      } else if (inst.funct3 == 0x4) {
        ans = rs1 ^ rs2;
      } else if (inst.funct3 == 0x1) {
        ans = rs1 << rs2;
      } else if (inst.funct3 == 0x5 && inst.funct7 == 0x0) {
        ans = rs1 >> rs2;
      } else if (inst.funct3 == 0x5 && inst.funct7 == 0x20) {
        ans = int32_t(rs1) >> rs2;
      } else if (inst.funct3 == 0x2) {
        ans = (int32_t(rs1) < int32_t(rs2)) ? 1 : 0;
      } else if (inst.funct3 == 0x3) {
        ans = (rs1 < rs2) ? 1 : 0;
      }
      res.value = ans;
    } else if (inst.opcode == 0x13) {
      uint32_t ans = 0;
      if (inst.funct3 == 0x0) {
        ans = rs1 + inst.imm;
      } else if (inst.funct3 == 0x7) {
        ans = rs1 & inst.imm;
      } else if (inst.funct3 == 0x6) {
        ans = rs1 | inst.imm;
      } else if (inst.funct3 == 0x4) {
        ans = rs1 ^ inst.imm;
      } else if (inst.funct3 == 0x1) {
        ans = rs1 << inst.imm;
      } else if (inst.funct3 == 0x5 && inst.funct7 == 0x0) {
        ans = rs1 >> inst.imm;
      } else if (inst.funct3 == 0x5 && inst.funct7 == 0x20) {
        ans = int32_t(rs1) >> inst.imm;
      } else if (inst.funct3 == 0x2) {
        ans = (int32_t(rs1) < int32_t(inst.imm)) ? 1 : 0;
      } else if (inst.funct3 == 0x3) {
        ans = rs1 < inst.imm ? 1 : 0;
      }
      res.value = ans;
    } else if (inst.opcode == 0x03) {
      uint32_t addr = rs1 + inst.imm;
      res.value = addr;
    } else if (inst.opcode == 0x23) {
      uint32_t addr = rs1 + inst.imm;
      res.value = addr;
    } else if (inst.opcode == 0x63) {
      res.is_branch = true;
      if (inst.funct3 == 0x0) {
        res.go_branch = (rs1 == rs2);
      } else if (inst.funct3 == 0x5) {
        res.go_branch = (int32_t(rs1) >= int32_t(rs2));
      } else if (inst.funct3 == 0x7) {
        res.go_branch = (rs1 >= rs2);
      } else if (inst.funct3 == 0x4) {
        res.go_branch = (int32_t(rs1) < int32_t(rs2));
      } else if (inst.funct3 == 0x6) {
        res.go_branch = (rs1 < rs2);
      } else if (inst.funct3 == 0x1) {
        res.go_branch = (rs1 != rs2);
      }
      res.next_pc = res.go_branch ? pc + inst.imm : pc + 4;
    } else if (inst.opcode == 0x6F) {
      res.value = pc + 4;
      res.is_branch = true;
      res.go_branch = true;
      res.next_pc = pc + inst.imm;
    } else if (inst.opcode == 0x67) {
      res.value = pc + 4;
      res.is_branch = true;
      res.go_branch = true;
      res.next_pc = (rs1 + inst.imm) & ~1u;
    } else if (inst.opcode == 0x17) {
      res.value = pc + inst.imm;
    } else if (inst.opcode == 0x37) {
      res.value = inst.imm;
    }
    new_res = res;
    new_result_ready = true;
    new_result_tag = old_data_tag;
  }
  void set_alu(const DecodedIns &inst, uint32_t rs1, uint32_t rs2, uint32_t pc,
               uint8_t tag) {
    new_inst = inst;
    new_rs1 = rs1;
    new_rs2 = rs2;
    new_pc = pc;
    new_data_tag = tag;
    new_data_ready = true;
  }
  ALUResult get_result() const { return old_res; }
  int get_result_tag() const { return old_result_ready ? old_result_tag : -1; }
  bool get_result_ready() const { return old_result_ready; }
  void tick() {
    old_inst = new_inst;
    old_rs1 = new_rs1;
    old_rs2 = new_rs2;
    old_pc = new_pc;
    old_res = new_res;
    old_data_tag = new_data_tag;
    old_result_tag = new_result_tag;
    old_data_ready = new_data_ready;
    old_result_ready = new_result_ready;

    new_data_ready = false;
    new_result_ready = false;
  }
};
