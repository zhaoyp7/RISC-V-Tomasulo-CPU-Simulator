#pragma once

#include "decoder.hpp"
#include <cstdint>

const int RS_SIZE = 8;

struct RSData {
  bool busy;
  DecodedIns inst;
  uint32_t Vj, Vk;
  uint8_t Qj, Qk;
  uint8_t tag;
  uint32_t pc;
  int wait_cycles;
  bool is_branch;
};

class ReservationStation {
private:
  RSData old_data[RS_SIZE];
  RSData new_data[RS_SIZE];

public:
  ReservationStation();
  int insert(const DecodedIns &inst, uint8_t tag, uint32_t Vj, uint32_t Vk, uint8_t Qj, uint8_t Qk, uint32_t pc);
  bool check_full();
  int find_ready();
  void execute();
  uint32_t get_result(int idx);
  void remove(int idx);
  void flush();
  uint8_t get_tag(int idx);
  void listen_cdb(uint8_t tag, uint32_t value);
  void tick();
};