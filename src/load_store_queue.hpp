#pragma once

#include "memory.hpp"
#include <cstdint>

const int LSQ_SIZE = 8;

enum class LSQType { LOAD, STORE };

struct LSQData {
  bool busy;
  LSQType type;
  uint8_t funct3;
  bool addr_ready, data_ready;
  uint32_t addr, data;
  uint8_t tag, store_data_tag, addr_tag;
  int wait_cycles;
};

class LoadStoreQueue {
private:
  LSQData old_data[LSQ_SIZE];
  LSQData new_data[LSQ_SIZE];

public:
  LoadStoreQueue() {
    for (int i = 0; i < LSQ_SIZE; i++) {
      old_data[i].busy = new_data[i].busy = false;
    }
  }
  int insert(uint8_t opcode, uint8_t tag, uint8_t funct3, uint32_t addr,
             bool addr_ready, uint32_t data, uint8_t store_data_tag,
             bool data_ready, uint8_t addr_tag) {
    for (int i = 0; i < LSQ_SIZE; i++) {
      if (new_data[i].busy == false) {
        LSQType type = (opcode == 0x03) ? LSQType::LOAD : LSQType::STORE;
        new_data[i] =
            (LSQData){true, type, funct3,         addr_ready, data_ready, addr,
                      data, tag,  store_data_tag, addr_tag,   3};
        return i;
      }
    }
    return -1;
  }
  bool check_full() {
    for (int i = 0; i < LSQ_SIZE; i++) {
      if (old_data[i].busy == false) {
        return false;
      }
    }
    return true;
  }
  void execute(Memory &mem) {
    for (int i = 0; i < LSQ_SIZE; i++) {
      if (!new_data[i].busy || !new_data[i].addr_ready) {
        continue;
      }
      if (new_data[i].type == LSQType::STORE && !new_data[i].data_ready) {
        continue;
      }
      if (new_data[i].wait_cycles) {
        new_data[i].wait_cycles--;
        if (new_data[i].wait_cycles == 0) {
          if (new_data[i].type == LSQType::LOAD) {
            commit_load(i, mem);
          }
        }
      }
    }
  }
  void commit_load(int idx, Memory &mem) {
    uint32_t addr = new_data[idx].addr;
    uint32_t ans = 0;
    uint8_t funct3 = new_data[idx].funct3;
    if (funct3 == 0x0) {
      ans = mem.load_byte(addr);
    } else if (funct3 == 0x4) {
      ans = mem.load_byte_unsigned(addr);
    } else if (funct3 == 0x1) {
      ans = mem.load_half_word(addr);
    } else if (funct3 == 0x5) {
      ans = mem.load_half_word_unsigned(addr);
    } else if (funct3 == 0x2) {
      ans = mem.load_word(addr);
    }
    new_data[idx].data = ans;
  }
  void commit_write(int idx, Memory &mem) {
    uint32_t addr = old_data[idx].addr;
    uint32_t val = old_data[idx].data;
    uint8_t funct3 = old_data[idx].funct3;
    if (funct3 == 0x0) {
      mem.store_byte(addr, val & 0xFF);
    } else if (funct3 == 0x1) {
      mem.store_half_word(addr, val & 0xFFFF);
    } else if (funct3 == 0x2) {
      mem.store_word(addr, val);
    }
  }
  uint32_t get_load_result(int idx) { return new_data[idx].data; }
  uint8_t get_tag(int idx) { return old_data[idx].tag; }
  bool is_load(int idx) { return (old_data[idx].type == LSQType::LOAD); }
  bool is_store(int idx) { return (old_data[idx].type == LSQType::STORE); }
  void remove(int idx) { new_data[idx].busy = false; }
  bool check_load_done(int idx) {
    if (!new_data[idx].busy || new_data[idx].type != LSQType::LOAD) {
      return false;
    }
    return (new_data[idx].addr_ready && new_data[idx].wait_cycles == 0);
  }
  void flush() {
    for (int i = 0; i < LSQ_SIZE; i++) {
      new_data[i].busy = false;
    }
  }
  void listen_cdb(uint8_t tag, uint32_t value) {
    for (int i = 0; i < LSQ_SIZE; i++) {
      if (new_data[i].busy) {
        if (!new_data[i].addr_ready && new_data[i].addr_tag == tag) {
          new_data[i].addr_ready = true;
          new_data[i].addr = value;
        }
        if (new_data[i].type == LSQType::STORE && !new_data[i].data_ready &&
            new_data[i].store_data_tag == tag) {
          new_data[i].data_ready = true;
          new_data[i].data = value;
        }
      }
    }
  }
  void tick() {
    for (int i = 0; i < LSQ_SIZE; i++) {
      old_data[i] = new_data[i];
    }
  }
};