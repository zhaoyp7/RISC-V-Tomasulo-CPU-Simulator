#pragma once

#include <cstdint>

enum InsType { R, I, S, B, J, U, Istar, UNKNOWN };

struct DecodedIns {
  InsType type;
  uint8_t opcode;
  uint8_t rd;
  uint8_t rs1, rs2;
  uint8_t funct3, funct7;
  uint32_t imm;
};

class Decoder {

public:
  DecodedIns decode(uint32_t addr) const {
    DecodedIns res;
    res.opcode = (addr & 0x7F);
    uint32_t tmp = (addr >> 11) & 0x7;
    if (res.opcode == 0x33) {
      res.type = InsType::R;
      res.rd = (addr >> 7) & 0x1F;
      res.funct3 = (addr >> 11) & 0x7;
      res.rs1 = (addr >> 15) & 0x1F;
      res.rs2 = (addr >> 20) & 0x1F;
      res.funct7 = (addr >> 25);
    } else if (res.opcode == 0x13 && (tmp == 0x1 || tmp == 0x5)) {
      res.type = InsType::Istar;
      res.rd = (addr >> 7) & 0x1F;
      res.funct3 = (addr >> 11) & 0x7;
      res.rs1 = (addr >> 15) & 0x1F;
      res.imm = (addr >> 20) & 0x1F;
      res.funct7 = (addr >> 25);
    } else if (res.opcode == 0x13 || res.opcode == 0x03 || res.opcode == 0x67 ||
               res.opcode == 0x73) {
      res.type = InsType::I;
      res.rd = (addr >> 7) & 0x1F;
      res.funct3 = (addr >> 11) & 0x7;
      res.rs1 = (addr >> 15) & 0x1F;
      res.imm = (int32_t(addr) >> 20);
    } else if (res.opcode == 0x23) {
      res.type = InsType::S;
      res.funct3 = (addr >> 11) & 0x7;
      res.rs1 = (addr >> 15) & 0x1F;
      res.rs2 = (addr >> 20) & 0x1F;
      res.imm = (addr >> 7) & 0x1F;
      res.imm |= (int32_t(addr) >> 25) << 5;
    } else if (res.opcode == 0x63) {
      res.type = InsType::B;
      res.funct3 = (addr >> 11) & 0x7;
      res.rs1 = (addr >> 15) & 0x1F;
      res.rs2 = (addr >> 20) & 0x1F;
      res.imm = (int32_t(addr) >> 31) << 12;
      res.imm |= ((addr >> 7) & 0x1) << 11;
      res.imm |= ((addr >> 25) & 0x3F) << 5;
      res.imm |= ((addr >> 8) & 0xF) << 1;
    } else if (res.opcode == 0x6F) {
      res.type = InsType::J;
      res.rd = (addr >> 7) & 0x1F;
      res.imm = (int32_t(addr) >> 31) << 20;
      res.imm |= ((addr >> 12) & 0xFF) << 12;
      res.imm |= ((addr >> 20) & 0x1) << 11;
      res.imm |= ((addr >> 21) & 0x3FF) << 1;
    } else if (res.opcode == 0x17 || res.opcode == 0x37) {
      res.type = InsType::U;
      res.rd = (addr >> 7) & 0x1F;
      res.imm = (addr >> 12) << 12;
    } else {
      res.type = InsType::UNKNOWN;
    }
    return res;
  }
};