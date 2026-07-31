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
  CommonDataBus() { old_broadcast.flag = new_broadcast.flag = false; }
  void broadcast(uint8_t tag, uint32_t value) {
    if (new_broadcast.flag) {
      return;
    }
    new_broadcast.flag = true;
    new_broadcast.tag = tag;
    new_broadcast.value = value;
  }
  bool has_broadcast() { return old_broadcast.flag; }
  CDBData get_broadcast() { return old_broadcast; }
  void tick() {
    old_broadcast = new_broadcast;
    new_broadcast.flag = false;
  }
};