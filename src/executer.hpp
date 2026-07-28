#pragma once

#include "decoder.hpp"
#include "memory.hpp"
#include "regfile.hpp"
#include <cstdint>

struct ExecuteResult {
  uint32_t next_pc;
  bool stop;
  uint32_t return_val;
};

class Executor {
public:
  ExecuteResult execute(const DecodedIns &inst, RegFile &reg, Memory &mem, uint32_t pc) const {
    ExecuteResult res;
    res.next_pc = pc + 4;
    res.stop = false;
    if (inst.opcode == 0x33) {
      uint32_t rs1 = reg.read(inst.rs1);
      uint32_t rs2 = reg.read(inst.rs2);
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
      reg.write(inst.rd, ans);
    } else if (inst.opcode == 0x13) {
      uint32_t rs1 = reg.read(inst.rs1);
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
      reg.write(inst.rd, ans);
    } else if (inst.opcode == 0x03) {
      uint32_t addr = inst.rs1 + inst.imm;
      uint32_t ans = 0;
      if (inst.funct3 == 0x0) {
        ans = mem.load_byte(addr);
      } else if (inst.funct3 == 0x4) {
        ans = mem.load_byte_unsigned(addr);
      } else if (inst.funct3 == 0x1) {
        ans = mem.load_half_word(addr);
      } else if (inst.funct3 == 0x5) {
        ans = mem.load_half_word_unsigned(addr);
      } else if (inst.funct3 == 0x2) {
        ans = mem.load_word(addr);
      }
      reg.write(inst.rd, ans);
    } else if (inst.opcode == 0x23) {
      uint32_t addr = inst.rs1 + inst.imm;
      uint32_t val = reg.read(inst.rs2);
      if (inst.funct3 == 0x0) {
        mem.store_byte(addr, val & 0xFF);
      } else if (inst.funct3 == 0x1) {
        mem.store_half_word(addr, val & 0xFFFF);
      } else if (inst.funct3 == 0x2) {
        mem.store_word(addr, val);
      }
    } else if (inst.opcode == 0x63) {
      uint32_t rs1 = reg.read(inst.rs1);
      uint32_t rs2 = reg.read(inst.rs2);
      if (inst.funct3 == 0x0) {
        res.next_pc = (rs1 == rs2) ? pc + inst.imm : pc + 4;
      } else if (inst.funct3 == 0x5) {
        res.next_pc = (int32_t(rs1) >= int32_t(rs2)) ? pc + inst.imm : pc + 4;
      } else if (inst.funct3 == 0x7) {
        res.next_pc = (rs1 >= rs2) ? pc + inst.imm : pc + 4;
      } else if (inst.funct3 == 0x4) {
        res.next_pc = (int32_t(rs1) < int32_t(rs2)) ? pc + inst.imm : pc + 4;
      } else if (inst.funct3 == 0x6) {
        res.next_pc = (rs1 < rs2) ? pc + inst.imm : pc + 4;
      } else if (inst.funct3 == 0x1) {
        res.next_pc = (rs1 != rs2) ? pc + inst.imm : pc + 4;
      }
    } else if (inst.opcode == 0x6F) {
      reg.write(inst.rd, pc + 4);
      res.next_pc = pc + inst.imm;
    } else if (inst.opcode == 0x67) {
      uint32_t rs1 = reg.read(inst.rs1);
      reg.write(inst.rd, pc + 4);
      res.next_pc = (rs1 + inst.imm) & ~1u;
    } else if (inst.opcode == 0x17) {
      reg.write(inst.rd, pc + inst.imm);
    } else if (inst.opcode == 0x37) {
      reg.write(inst.rd, inst.imm);
    }
    return res;
  }
};