#pragma once

#include <cstdint>

struct CDBData {
  bool flag;
  uint8_t tag;
  uint32_t value;
};

class CommonDataBus {
private:
  CDBData old_broadcast;
  CDBData new_broadcast;

public:
  CommonDataBus();
  void broadcast(uint8_t tag, uint32_t value);
  bool has_broadcast();
  CDBData get_broadcast();
  void tick();
};