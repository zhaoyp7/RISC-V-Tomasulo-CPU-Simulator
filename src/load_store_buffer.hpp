#pragma once

#include <cstdint>
#include "memory.hpp"

const int LSB_SIZE = 8;

enum class LSBType {LOAD, STORE};

struct LSBData {
  bool busy;
  LSBType type;
  uint8_t funct3;
  bool addr_ready, data_ready;
  uint32_t addr, data;
  uint8_t tag, store_data_tag;
  int wait_cycles;
};

class LoadStoreBuffer {
private:
  LSBData old_data[LSB_SIZE];
  LSBData new_data[LSB_SIZE];

public:
  LoadStoreBuffer();
  int insert(uint8_t opcode, uint8_t tag, uint8_t funct3, uint32_t addr, uint32_t addr_ready, uint32_t data, uint8_t store_data_tag, bool data_ready);
  bool check_full();
  void execute(Memory &mem);
  void commit_write(int idx, Memory &mem);
  uint32_t get_load_result(int idx);
  uint8_t get_tag(int idx);
  bool is_load(int idx);
  bool is_store(int idx);
  bool check_load_done(int idx);
  void remove(int idx);
  void flush();
  void listen_cdb(uint8_t tag, uint32_t value);
  void tick();
};