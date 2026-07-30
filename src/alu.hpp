#pragma once

#include <cstdint>
#include "decoder.hpp"

struct ALUResult {
  uint32_t value;
  uint32_t next_pc;
  uint32_t go_branch;
  uint32_t is_branch;
};

class ALU {
public:
  static ALUResult execute(const DecodedIns &inst, uint32_t rs1, uint32_t rs2, uint32_t pc) {
    ALUResult res;
    res.value = 0;
    res.next_pc = pc + 4;
    res.is_branch = res.go_branch = 0;
    if (inst.opcode == 0x33) {
      uint32_t ans = 0;
      if (inst.funct3 == 0x0) {
        ans = (inst.funct7 == 0x0) ? (rs1 + rs2) : (rs1 - rs2);
      } else  if (inst.funct3 == 0x7) {
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
    return res;
  }
};
