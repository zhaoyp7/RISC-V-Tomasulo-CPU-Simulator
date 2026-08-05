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
  int old_lsq_idx, new_lsq_idx;
  uint32_t old_addr, new_addr;
  uint32_t old_val, new_val;
  uint8_t old_lsq_tag, new_lsq_tag;
  int old_len, new_len;

public:
  Memory() {
    old_lsq_idx = new_lsq_idx = -1;
    old_addr = new_addr = 0;
    old_val = new_val = 0;
    old_len = new_len = 0;
    old_lsq_tag = new_lsq_tag = 0;
  }
  void init() {
    std::string str;
    uint32_t cur = 0;
    while (std::cin >> str) {
      if (str[0] == '@') {
        cur = 0;
        for (int i = 1; i < (int) str.size(); i++) {
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
    mem[addr + 1] = (val >> 8) & 0xFF;
    mem[addr + 2] = (val >> 16) & 0xFF;
    mem[addr + 3] = (val >> 24);
  }
  void set_store(uint32_t addr, uint32_t val, int lsq_idx, int len, uint8_t tag) {
    new_addr = addr;
    new_val = val;
    new_len = len;
    new_lsq_idx = lsq_idx;
    new_lsq_tag = tag;
    // if (len == 1) {
    //   store_byte(addr, val);
    // } else if (len == 2) {
    //   store_half_word(addr, val);
    // } else if (len == 4) {
    //   store_word(addr, val);
    // }
  }
  void run() {
    if (old_lsq_idx == -1) {
      return ;
    }
    if (old_len == 1) {
      store_byte(old_addr, old_val);
    } else if (old_len == 2) {
      store_half_word(old_addr, old_val);
    } else if (old_len == 4) {
      store_word(old_addr, old_val);
    }
  }
  void tick() {
    old_addr = new_addr;
    old_val = new_val;
    old_lsq_idx = new_lsq_idx;
    old_len = new_len;
    old_lsq_tag = new_lsq_tag;
    new_lsq_idx = -1;
  }
  int get_lsq_idx() { return old_lsq_idx; }
  int get_lsq_tag() { return old_lsq_tag; }
};
