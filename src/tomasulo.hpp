#pragma once

#include "alu.hpp"
#include "branch_predictor.hpp"
#include "cdb.hpp"
#include "decoder.hpp"
#include "fetch.hpp"
#include "load_store_queue.hpp"
#include "memory.hpp"
#include "regfile.hpp"
#include "reorder_buffer.hpp"
#include "reservation_station.hpp"
#include <cstdint>

class Tomasulo {
private:
  Decoder decoder;
  RegFile reg;
  Memory mem;
  Fetch fetch;
  ReorderBuffer rob;
  CommonDataBus cdb;
  ReservationStation rs;
  LoadStoreQueue lsq;
  BranchPredictor bp;
  bool halt;
  int cycles;

  void commit_stage() {
    while (rob.check_commit()) {
      ROBData data = rob.commit();
      if (data.inst.opcode == 0x23) {
        lsq.commit_write(data.lsq_idx, mem);
      } else if (data.inst.opcode != 0x63) {
        reg.write(data.dest, data.value);
      }
      if (data.is_branch) {
        bp.predict(data.pc);
        bp.update(data.pc, data.go_branch);
        bool flag = (data.pred_pc != data.pc + 4);
        if (flag != data.go_branch) {
          halt = false;
          rob.flush_all();
          rs.flush();
          lsq.flush();
          fetch.recover_pc(data.actual_pc);
        }
      }
    }
  }
  void writeback_stage() {
    for (int i = 0; i < LSQ_SIZE; i++) {
      if (lsq.check_load_done(i)) {
        uint32_t value = lsq.get_load_result(i);
        uint8_t tag = lsq.get_tag(i);
        cdb.broadcast(tag, value);
        rob.set_write(tag, value);
        lsq.remove(i);
        return;
      }
    }
    for (int i = 0; i < RS_SIZE; i++) {
      if (rs.check_done(i)) {
        ALUResult alu = rs.get_result(i);
        uint8_t tag = rs.get_tag(i);
        cdb.broadcast(tag, alu.value);
        if (alu.is_branch) {
          uint32_t actual_pc = alu.go_branch ? alu.next_pc : rs.get_pc(i) + 4;
          rob.set_branch(tag, alu.go_branch, actual_pc);
        }
        rob.set_write(tag, alu.value);
        rs.remove(i);
        return;
      }
    }
  }
  void cdb_listen_stage() {
    if (!cdb.has_broadcast()) {
      return;
    }
    CDBData data = cdb.get_broadcast();
    rs.listen_cdb(data.tag, data.value);
    lsq.listen_cdb(data.tag, data.value);
    reg.update_from_cdb(data.tag, data.value);
  }
  void execute_stage() {
    rs.execute();
    lsq.execute(mem);
  }
  void issue_stage() {
    if (halt || rs.check_full() || lsq.check_full() || rob.check_full()) {
      return ;
    }
    uint32_t ins, pred_pc;
    fetch.fetch(ins, pred_pc);
    uint32_t pc = fetch.get_pc();
    if (ins == 0x0ff00513) {
      halt = true;
      return ;
    }
    DecodedIns inst = decoder.decode(ins);
    uint32_t Vj = 0, Vk = 0;
    uint8_t Qj = 0, Qk = 0;
    if (need_rs1(inst)) {
      RegStatus tmp = reg.get_status(inst.rs1);
      if (tmp.ready) {
        Vj = tmp.value;
      } else if (cdb.has_broadcast() && cdb.get_broadcast().tag == tmp.tag) {
        Vj = cdb.get_broadcast().value;
      } else {
        Qj = tmp.tag;
      }
    }
    if (need_rs2(inst)) {
      RegStatus tmp = reg.get_status(inst.rs2);
      if (tmp.ready) {
        Vk = tmp.value;
      } else if (cdb.has_broadcast() && cdb.get_broadcast().tag == tmp.tag) {
        Vk = cdb.get_broadcast().value;
      } else {
        Qk = tmp.tag;
      }
    }
    int tag = rob.insert(inst, pc, pred_pc);
    if (need_rd(inst)) {
      reg.set_waiting(inst.rd, tag);
    }
    if (inst.opcode == 0x03) {
      uint32_t addr = (Qj == 0) ? Vj + inst.imm : 0;
      int lsq_idx = lsq.insert(inst.opcode, tag, inst.funct3, addr, (Qj == 0), 0, 0, true, Qj, inst.imm);
      rob.set_lsq_idx(tag, lsq_idx);
    } else if (inst.opcode == 0x23) {
      uint32_t addr = (Qj == 0) ? Vj + inst.imm : 0;
      int lsq_idx = lsq.insert(inst.opcode, tag, inst.funct3, addr, (Qj == 0), Vk , Qk, (Qk == 0), Qj, inst.imm);
      rob.set_lsq_idx(tag, lsq_idx);
    } else {
      int rs_idx = rs.insert(inst, tag, Vj, Vk, Qj, Qk, pc);
    }
  }
  void tick_stage() {
    reg.tick();
    rob.tick();
    cdb.tick();
    rs.tick();
    lsq.tick();
  }
  bool need_rs1(const DecodedIns &inst) {
    return inst.opcode == 0x33 || inst.opcode == 0x13 || inst.opcode == 0x03 ||
           inst.opcode == 0x23 || inst.opcode == 0x63 || inst.opcode == 0x67;
  }
  bool need_rs2(const DecodedIns &inst) {
    return inst.opcode == 0x33 || inst.opcode == 0x23 || inst.opcode == 0x63;
  }
  bool need_rd(const DecodedIns &inst) {
    return inst.opcode == 0x33 || inst.opcode == 0x13 ||
               inst.opcode == 0x03 || inst.opcode == 0x6F ||
               inst.opcode == 0x67 || inst.opcode == 0x17 ||
               inst.opcode == 0x37;
      }

public:
  Tomasulo() : fetch(mem, bp) {
    halt = false;
    cycles = 0;
  }
  void init() { mem.init(); }
  bool check_done() { return (halt && rob.check_empty()); }
  uint32_t get_result() { return (reg.read(10) & 0xFF); }
  int get_cycles() { return cycles; }
  void bp_result() { bp.debug(); }
  void step() {
    cycles++;
    commit_stage();
    writeback_stage();
    cdb_listen_stage();
    execute_stage();
    issue_stage();
    tick_stage();
  }
};