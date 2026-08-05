#pragma once

#include <cstdint>

struct CDBData {
  bool flag;
  uint8_t tag;
  uint32_t value;
};

class CommonDataBus {
private:
  CDBData bc;

public:
  CommonDataBus() { bc.flag = false; }
  void broadcast(uint8_t tag, uint32_t value) {
    bc.flag = true;
    bc.tag = tag;
    bc.value = value;
  }
  bool has_broadcast() const { return bc.flag; }
  CDBData get_broadcast() const { return bc; }
  void tick() { bc.flag = false; }
};