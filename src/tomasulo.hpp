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
  ALU alu;
  bool halt;
  int cycles;
  uint32_t count;

  void Memory_stage() {
    mem.run();
    int lsq_idx = mem.get_lsq_idx();
    uint8_t lsq_tag = mem.get_lsq_tag();
    if (lsq_idx != -1 && lsq.get_tag(lsq_idx) == lsq_tag) {
      lsq.remove(lsq_idx);
    }
  }
  void ROB_stage(bool &flushed) {
    if (!rob.check_commit() || flushed) {
      return;
    }
    ROBData data = rob.commit();
    if (data.inst.opcode == 0x23) {
      lsq.commit_write(data.lsq_idx, mem);
    } else if (data.inst.opcode != 0x63) {
      reg.write(data.dest, data.value);
    }
    if (data.is_branch) {
      bool flag = (data.pred_pc != data.pc + 4);
      bp.update(data.pc, data.go_branch, flag);
      if (flag != data.go_branch) {
        halt = false;
        flushed = true;
        rob.flush();
        rs.flush();
        lsq.flush();
        reg.flush();
        fetch.recover_pc(data.actual_pc);
      }
    }
  }
  void RS_stage(bool flushed) {
    if (flushed) {
      return;
    }
    rs.execute();
    for (int i = 0; i < RS_SIZE; i++) {
      if (rs.check_done(i)) {
        alu.set_alu(rs.get_inst(i), rs.get_Vj(i), rs.get_Vk(i), rs.get_pc(i),
                    rs.get_tag(i));
        break;
      }
    }
    if (cdb.has_broadcast()) {
      return;
    }
    for (int i = 0; i < RS_SIZE; i++) {
      if (alu.get_result_tag() == rs.get_tag(i) && rs.check_done(i)) {
        ALUResult res = alu.get_result();
        uint8_t tag = rs.get_tag(i);
        cdb.broadcast(tag, res.value);
        if (res.is_branch) {
          uint32_t actual_pc = res.go_branch ? res.next_pc : rs.get_pc(i) + 4;
          rob.set_branch(tag, res.go_branch, actual_pc);
        }
        rob.set_write(tag, res.value);
        rs.remove(i);
        cdb_listen_stage(0);
        return;
      }
    }
  }
  void LSQ_stage(bool flushed) {
    if (flushed) {
      return;
    }
    lsq.execute(mem);
    for (int i = 0; i < LSQ_SIZE; i++) {
      if (lsq.check_store_done(i)) {
        rob.set_write(lsq.get_tag(i), 0);
      }
    }
    if (cdb.has_broadcast() || alu.get_result_ready()) {
      return;
    }
    for (int i = 0; i < LSQ_SIZE; i++) {
      if (lsq.check_load_done(i)) {
        uint32_t value = lsq.get_load_result(i);
        uint8_t tag = lsq.get_tag(i);
        cdb.broadcast(tag, value);
        rob.set_write(tag, value);
        lsq.remove(i);
        cdb_listen_stage(0);
        return;
      }
    }
  }
  void cdb_listen_stage(bool flushed) {
    if (!cdb.has_broadcast() || flushed) {
      return;
    }
    CDBData data = cdb.get_broadcast();
    rs.listen_cdb(data.tag, data.value);
    lsq.listen_cdb(data.tag, data.value);
    reg.update_from_cdb(data.tag, data.value);
  }
  void Fetch_stage(bool flushed) {
    if (halt || rs.check_full() || lsq.check_full() || rob.check_full()) {
      return;
    }
    if (flushed) {
      return;
    }
    uint32_t ins, pred_pc;
    fetch.fetch(ins, pred_pc);
    uint32_t pc = fetch.get_pc();
    if (ins == 0x0ff00513) {
      halt = true;
      return;
    }
    DecodedIns inst = decoder.decode(ins);
    uint32_t Vj = 0, Vk = 0;
    uint8_t Qj = 0, Qk = 0;
    if (need_rs1(inst)) {
      RegStatus tmp = reg.get_status(inst.rs1);
      if (tmp.ready) {
        Vj = tmp.value;
      } else {
        Qj = tmp.tag;
      }
    }
    if (need_rs2(inst)) {
      RegStatus tmp = reg.get_status(inst.rs2);
      if (tmp.ready) {
        Vk = tmp.value;
      } else {
        Qk = tmp.tag;
      }
    }
    int tag = rob.insert(inst, pc, pred_pc);
    if (need_rd(inst)) {
      reg.set_tag(inst.rd, tag);
    }
    if (inst.opcode == 0x03) {
      uint32_t addr = (Qj == 0) ? Vj + inst.imm : 0;
      int lsq_idx = lsq.insert(inst.opcode, tag, inst.funct3, addr, (Qj == 0),
                               0, 0, true, Qj, inst.imm, count++);
      rob.set_lsq_idx(tag, lsq_idx);
    } else if (inst.opcode == 0x23) {
      uint32_t addr = (Qj == 0) ? Vj + inst.imm : 0;
      int lsq_idx = lsq.insert(inst.opcode, tag, inst.funct3, addr, (Qj == 0),
                               Vk, Qk, (Qk == 0), Qj, inst.imm, count++);
      rob.set_lsq_idx(tag, lsq_idx);
    } else {
      int rs_idx = rs.insert(inst, tag, Vj, Vk, Qj, Qk, pc);
    }
  }
  void tick_stage(bool flushed) {
    reg.tick();
    rob.tick();
    rs.tick();
    lsq.tick();
    alu.tick();
    mem.tick();
    cdb_listen_stage(flushed);
    cdb.tick();
  }
  bool need_rs1(const DecodedIns &inst) {
    return inst.opcode == 0x33 || inst.opcode == 0x13 || inst.opcode == 0x03 ||
           inst.opcode == 0x23 || inst.opcode == 0x63 || inst.opcode == 0x67;
  }
  bool need_rs2(const DecodedIns &inst) {
    return inst.opcode == 0x33 || inst.opcode == 0x23 || inst.opcode == 0x63;
  }
  bool need_rd(const DecodedIns &inst) {
    return inst.opcode == 0x33 || inst.opcode == 0x13 || inst.opcode == 0x03 ||
           inst.opcode == 0x6F || inst.opcode == 0x67 || inst.opcode == 0x17 ||
           inst.opcode == 0x37;
  }

public:
  Tomasulo() : fetch(mem, bp) {
    halt = false;
    cycles = 0;
    count = 0;
  }
  void init() { mem.init(); }
  bool check_done() { return (halt && rob.check_empty()); }
  uint32_t get_result() { return (reg.read(10) & 0xFF); }
  int get_cycles() { return cycles; }
  void bp_result() { bp.debug(); }
  void step() {
    cycles++;
    bool flushed = false;

    alu.run();
    ROB_stage(flushed);
    Fetch_stage(flushed);
    LSQ_stage(flushed);
    RS_stage(flushed);
    Memory_stage();

    tick_stage(flushed);
  }
};
