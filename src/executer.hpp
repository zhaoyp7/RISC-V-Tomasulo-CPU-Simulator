#pragma once

#include "decoder.hpp"
#include "memory.hpp"
#include "regfile.hpp"
#include "alu.hpp"
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
    uint32_t rs1 = reg.read(inst.rs1);
    uint32_t rs2 = reg.read(inst.rs2);
    ALUResult alu = ALU::execute(inst, rs1, rs2, pc);

    if (inst.opcode == 0x33) {
      reg.write(inst.rd, alu.value);
    } else if (inst.opcode == 0x13) {
      reg.write(inst.rd, alu.value);
    } else if (inst.opcode == 0x03) {
      uint32_t addr = alu.value;
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
      uint32_t addr = alu.value;
      uint32_t val = rs2;
      if (inst.funct3 == 0x0) {
        mem.store_byte(addr, val & 0xFF);
      } else if (inst.funct3 == 0x1) {
        mem.store_half_word(addr, val & 0xFFFF);
      } else if (inst.funct3 == 0x2) {
        mem.store_word(addr, val);
      }
    } else if (inst.opcode == 0x63) {  
      res.next_pc = alu.next_pc;
    } else if (inst.opcode == 0x6F) {
      reg.write(inst.rd, alu.value);
      res.next_pc = alu.next_pc;
    } else if (inst.opcode == 0x67) {
      reg.write(inst.rd, alu.value);
      res.next_pc = alu.next_pc;
    } else if (inst.opcode == 0x17) {
      reg.write(inst.rd, alu.value);
    } else if (inst.opcode == 0x37) {
      reg.write(inst.rd, alu.value);
    }
  
    return res;
  }
};