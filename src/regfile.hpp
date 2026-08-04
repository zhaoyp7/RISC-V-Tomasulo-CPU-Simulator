#pragma once

#include <cstdint>
#include <cstdio>

struct RegStatus {
  bool ready;
  uint32_t value;
  uint8_t tag;
};

class RegFile {
private:
  uint32_t old_reg[32], new_reg[32];
  RegStatus old_status[32], new_status[32];

public:
  RegFile() {
    for (int i = 0; i < 32; i++) {
      old_reg[i] = new_reg[i] = 0;
      old_status[i] = new_status[i] = (RegStatus){1, 0, 0};
    }
  }
  uint32_t read(uint8_t addr) const { return old_reg[addr]; }
  void write(uint8_t addr, uint32_t val) {
    if (addr == 0) {
      return;
    }
    new_reg[addr] = val;
  }
  RegStatus get_status(uint8_t addr) const { return old_status[addr]; }
  void set_waiting(uint8_t addr, uint8_t waiting_tag) {
    if (addr == 0) {
      return;
    }
    new_status[addr] = (RegStatus){0, 0, waiting_tag};
  }
  void update_from_cdb(uint8_t waiting_tag, uint32_t value) {
    for (int i = 0; i < 32; i++) {
      if (!old_status[i].ready && old_status[i].tag != 0 &&
          old_status[i].tag == waiting_tag) {
        new_status[i] = (RegStatus){1, value, 0};
      }
    }
  }
  void flush() {
    for (int i = 0; i < 32; i++) {
      new_status[i] = (RegStatus){true, old_reg[i], 0};
    }
  }
  void tick() {
    for (int i = 0; i < 32; i++) {
      old_reg[i] = new_reg[i];
      old_status[i] = new_status[i];
    }
  }
  void debug() {
    puts("REG debug");
    puts("OLD DEBUG");
    for (int i = 0; i < 32; i++) printf("%u %d %u %u\n",old_reg[i],old_status[i].ready,old_status[i].tag,old_status[i].value);
    puts("NEW DEBUG");
    for (int i = 0; i < 32; i++) printf("%u %d %u %u\n",new_reg[i],new_status[i].ready,new_status[i].tag,new_status[i].value);
  }
};