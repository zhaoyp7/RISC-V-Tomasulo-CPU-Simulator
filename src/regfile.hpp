#pragma once

#include <cstdint>

class RegFile {
private:
  uint32_t old_reg[32], new_reg[32];

public:
  RegFile() {
    for (int i = 0; i < 32; i++) {
      old_reg[i] = new_reg[i] = 0;
    }
  }
  uint32_t read(uint8_t addr) const { return old_reg[addr]; }
  void write(uint8_t addr, uint32_t val) {
    if (addr == 0) {
      return;
    }
    new_reg[addr] = val;
  }
  void tick() {
    for (int i = 0; i < 32; i++) {
      old_reg[i] = new_reg[i];
    }
  }
};