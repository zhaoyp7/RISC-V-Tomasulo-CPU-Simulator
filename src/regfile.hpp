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
  RegStatus old_status[32], new_cdb_status[32], new_tag_status[32];
  bool new_cdb_update[32], new_tag_update[32];

public:
  RegFile() {
    for (int i = 0; i < 32; i++) {
      old_reg[i] = new_reg[i] = 0;
      old_status[i] = new_cdb_status[i] = new_tag_status[i] =
          (RegStatus){1, 0, 0};
      new_cdb_update[i] = new_tag_update[i] = false;
    }
  }
  uint32_t read(uint8_t idx) const { return old_reg[idx]; }
  void write(uint8_t idx, uint32_t val) {
    if (idx == 0) {
      return;
    }
    new_reg[idx] = val;
  }
  RegStatus get_status(uint8_t idx) const { return old_status[idx]; }
  void set_tag(uint8_t idx, uint8_t tag) {
    if (idx == 0) {
      return;
    }
    new_tag_update[idx] = true;
    new_cdb_update[idx] = false;
    new_tag_status[idx] = (RegStatus){false, 0, tag};
  }
  void update_from_cdb(uint8_t tag, uint32_t value) {
    for (int i = 0; i < 32; i++) {
      if (!old_status[i].ready && old_status[i].tag != 0 && old_status[i].tag == tag) {
        new_cdb_update[i] = true;
        new_cdb_status[i] = (RegStatus){true, value, 0};
      }
    }
  }
  void flush() {
    for (int i = 0; i < 32; i++) {
      new_cdb_status[i] = new_tag_status[i] = (RegStatus){true, old_reg[i], 0};
      new_cdb_update[i] = new_tag_update[i] = true;
    }
  }
  void tick() {
    for (int i = 0; i < 32; i++) {
      old_reg[i] = new_reg[i];
      if (new_tag_update[i]) {
        old_status[i] = new_tag_status[i];
      } else if (new_cdb_update[i]) {
        old_status[i] = new_cdb_status[i];
      }
      new_tag_update[i] = new_cdb_update[i] = false;
    }
  }
  void debug() {
    puts("REG debug");
    puts("OLD DEBUG");
    for (int i = 0; i < 32; i++)
      printf("%u %d %u %u\n", old_reg[i], old_status[i].ready,
             old_status[i].tag, old_status[i].value);
    puts("NEW CDB DEBUG");
    for (int i = 0; i < 32; i++)
      printf("%u %d %u %u\n", new_reg[i], new_cdb_status[i].ready,
             new_cdb_status[i].tag, new_cdb_status[i].value);
    puts("NEW TAG DEBUG");
    for (int i = 0; i < 32; i++)
      printf("%u %d %u %u\n", new_reg[i], new_tag_status[i].ready,
             new_tag_status[i].tag, new_tag_status[i].value);
  }
};