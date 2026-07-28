#pragma once

#include <cstdint>
#include <iostream>
#include <map>
#include <string>

inline int hex_char_to_val(char ch) {
  if (ch >= '0' && ch <= '9') {
    return ch - '0';
  } else if (ch >= 'a' && ch <= 'f') {
    return ch - 'a' + 10;
  } else if (ch >= 'A' && ch <= 'F') {
    return ch - 'A' + 10;
  }
  return -1;
}

class Memory {
private:
  std::map<uint32_t, uint8_t> mem;

public:
  Memory() = default;
  void init() {
    std::string str;
    uint32_t cur = 0;
    while (std::cin >> str) {
      if (str[0] == '@') {
        cur = 0;
        for (int i = 1; i < str.size(); i++) {
          cur = (cur << 4) + hex_char_to_val(str[i]);
        }
        continue;
      }
      uint32_t byte = (hex_char_to_val(str[0]) << 4) | hex_char_to_val(str[1]);
      mem[cur++] = byte;
    }
  }
  uint32_t load_byte(uint32_t addr) {
    uint32_t res = mem[addr];
    if (res & 0x80) {
      res |= 0xFFFFFF00;
    }
    return res;
  }
  uint32_t load_byte_unsigned(uint32_t addr) {
    uint32_t res = mem[addr];
    return res;
  }
  uint32_t load_half_word(uint32_t addr) {
    uint32_t res = mem[addr] | (mem[addr + 1] << 8);
    if (res & 0x8000) {
      res |= 0XFFFF0000;
    }
    return res;
  }
  uint32_t load_half_word_unsigned(uint32_t addr) {
    uint32_t res = mem[addr] | (mem[addr + 1] << 8);
    return res;
  }
  uint32_t load_word(uint32_t addr) {
    uint32_t res = mem[addr] | (mem[addr + 1] << 8) | (mem[addr + 2] << 16) |
                   (mem[addr + 3] << 24);
    return res;
  }
  void store_byte(uint32_t addr, uint8_t val) { mem[addr] = val; }
  void store_half_word(uint32_t addr, uint16_t val) {
    mem[addr] = (val & 0xFF);
    mem[addr + 1] = (val >> 8);
  }
  void store_word(uint32_t addr, uint32_t val) {
    mem[addr] = (val & 0xFF);
    mem[addr] = (val >> 8) & 0xFF;
    mem[addr] = (val >> 16) & 0xFF;
    mem[addr] = (val >> 24);
  }
};
