#ifndef __NODE__H
#define __NODE__H

#include <mc_connections.h>
#include <systemc.h>

#include "defines.h"
#include "drim4hls.h"
#include "drim4hls_datatypes.h"
#include "globals.h"
#include "packet.h"

#define MEM_SIZE 256
// Add a DMEM address for the rank result
#define DMEM_RANK_ADDR (0x150 >> 2)

/**
 * SchedulingNode class
 * Implements a scheduling node for a network-on-chip (NoC) architecture.
 * Handles packet enqueue/dequeue operations, memory management, and scheduling
 * logic.
 *
 * A process for the DMEM and IMEM is included in order to simulate the CPU's
 * memory interface.
 */
#pragma hls_design top
SC_MODULE(SchedulingNode) {
  // Clock & reset
  sc_in<bool> clk;
  sc_in<bool> rst;

  // NoC interface ports
  Connections::In<packet_metadata_t> CCS_INIT_S1(in_pkt);
  Connections::Out<packet_metadata_t> CCS_INIT_S1(out_pkt);

  // IMEM and DMEM
  sc_uint<XLEN> imem[ICACHE_SIZE];
  sc_uint<XLEN> dmem[DCACHE_SIZE];

  // IMEM runtime write interface
  Connections::In<imem_write_req_t> CCS_INIT_S1(imem_write_port);

  // Channels for memory primitive interface
  Connections::Out<packet_enqueue_t> CCS_INIT_S1(
      mem_primitive_enqueue_ch);  // enqueue: metadata+rank
  Connections::Out<packet_dequeue_req_t> CCS_INIT_S1(
      mem_primitive_dequeue_req_ch);  // dequeue request: rank
  Connections::In<packet_dequeue_resp_t> CCS_INIT_S1(
      mem_primitive_dequeue_resp_ch);  // dequeue response: metadata

  // Connections channels for CPU
  Connections::Combinational<imem_out_t> imem2de_ch;
  Connections::Combinational<imem_in_t> fe2imem_ch;
  Connections::Combinational<dmem_out_t> dmem2wb_ch;
  Connections::Combinational<dmem_in_t> wb2dmem_ch;
  Connections::Combinational<packet_metadata_t> pkt2dmem_ch;

  /* CPU IS REMOVED FOR THE NODE'S SYNTH */
  /* FOR CPU SYNTH RESULTS, RUN A CPU ONLY SYNTH (See README)*/
  // CPU instance
  // drim4hls m_dut;

  // Instruction counters and program_end
  sc_signal<bool> program_end;
  sc_signal<long int> icount, j_icount, b_icount, m_icount, o_icount;

  // Internal state for scheduling node
  packet_metadata_t memory[MEM_SIZE];
  // Parent registers
  sc_uint<4> parent_id[2];
  // Scheduling registers
  sc_uint<32> scheduling_registers[32];

  sc_signal<bool> rank_ready;  // Flag to indicate rank is ready
  sc_uint<32> rank_value;      // Store the computed rank

  SC_HAS_PROCESS(SchedulingNode);
  SchedulingNode(sc_module_name name)
      : clk("clk"),
        rst("rst"),
        in_pkt("in_pkt"),
        out_pkt("out_pkt"),
        imem2de_ch("imem2de_ch"),
        fe2imem_ch("fe2imem_ch"),
        dmem2wb_ch("dmem2wb_ch"),
        wb2dmem_ch("wb2dmem_ch"),
        // m_dut("drim4hls"),
        mem_primitive_enqueue_ch("mem_primitive_enqueue_ch"),
        mem_primitive_dequeue_req_ch("mem_primitive_dequeue_req_ch"),
        mem_primitive_dequeue_resp_ch("mem_primitive_dequeue_resp_ch") {
    /* CPU IS REMOVED FOR THE NODE'S SYNTH */
    /* FOR CPU SYNTH RESULTS, RUN A CPU ONLY SYNTH (See README)*/
    // Connect CPU ports to local signals/channels
    // m_dut.clk(clk);
    // m_dut.rst(rst);

    // m_dut.program_end(program_end);

    // m_dut.icount(icount);
    // m_dut.j_icount(j_icount);
    // m_dut.b_icount(b_icount);
    // m_dut.m_icount(m_icount);
    // m_dut.o_icount(o_icount);

    // m_dut.imem2de_data(imem2de_ch);
    // m_dut.fe2imem_data(fe2imem_ch);
    // m_dut.dmem2wb_data(dmem2wb_ch);
    // m_dut.wb2dmem_data(wb2dmem_ch);
    program_end.write(true);

    SC_CTHREAD(node_th, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(imemory_th, clk.pos());
    async_reset_signal_is(rst, false);

    SC_CTHREAD(dmemory_th, clk.pos());
    async_reset_signal_is(rst, false);
  }

  void node_th() {
    in_pkt.Reset();
    out_pkt.Reset();
    wait();

    while (true) {
      if (rst.read() == false) {
        rank_ready.write(false);
      } else {
        // ENQ part
        // Normal packet handling
        packet_metadata_t pkt;
        if (in_pkt.PopNB(pkt)) {
          pkt2dmem_ch.Push(pkt);
        }

        // Only act when program_end transitions from 0 to 1 (rising edge)
        static bool prev_program_end = false;
        bool curr_program_end = program_end.read();

        if (curr_program_end && !prev_program_end) {
          // Fetch rank
          // rank_value = dmem[DMEM_RANK_ADDR];
          rank_value = 0;

          // Enqueue to primitive: send metadata + rank (here rank_value, or
          // compute as needed)
          packet_enqueue_t enq;
          enq.metadata = pkt;
          enq.rank = rank_value;
          mem_primitive_enqueue_ch.Push(enq);
        }
        prev_program_end = curr_program_end;

        // DQ part
        packet_dequeue_req_t deq_req;
        deq_req.rank = 0;  // Need to change based on primitive's strategy
        mem_primitive_dequeue_req_ch.Push(deq_req);  // Send dequeue request
        // Find a packet with matching rank (simple example: first match)
        packet_dequeue_resp_t deq_resp;

        // TO BE IMPLEMENTED with the desired memory primitive

        if (mem_primitive_dequeue_resp_ch.PopNB(deq_resp)) {
          out_pkt.Push(deq_resp.metadata);
        }
      }
      wait();
    }
  }

  void imemory_th() {
    imem2de_ch.ResetWrite();
    fe2imem_ch.ResetRead();
    imem_write_port.Reset();
    wait();

    while (true) {
      if (rst.read() == false) {
        // clear IMEM on reset
      } else {
        // Reconfiguration of the scheduler
        // Only allowed once the program is completed
        // Even if looping the program, it will set/reset program end
        if (program_end.read()) {
          imem_write_req_t req;
          if (imem_write_port.PopNB(req)) {
            unsigned addr = req.addr >> 2;
            if (addr < ICACHE_SIZE) {
              imem[addr] = req.data;
              wait();  // Prioritize IMEM write
              continue;
            }
          }
        }

        imem_in_t imem_in;
        if (fe2imem_ch.PopNB(imem_in)) {
          unsigned int addr_aligned = imem_in.instr_addr >> 2;
          imem_out_t imem_dout;
          imem_dout.instr_data = imem[addr_aligned];
          imem2de_ch.Push(imem_dout);
        }
      }
      wait();
    }
  }

  void dmemory_th() {
    wb2dmem_ch.ResetRead();
    dmem2wb_ch.ResetWrite();
    wait();

    while (true) {
      if (rst.read() == false) {
        // Clear DMEM on reset
      } else {
        packet_metadata_t pkt;
        if (pkt2dmem_ch.PopNB(pkt)) {
          unsigned base_addr = 0x100 >> 2;
          dmem[base_addr + 0] = pkt.src;
          dmem[base_addr + 1] = pkt.dst;
          dmem[base_addr + 2] = (pkt.length & 0xFFFF) |
                                ((pkt.tos & 0xFF) << 16) |
                                ((pkt.priority & 0x7) << 24);
          dmem[base_addr + 3] =
              (pkt.flow_id & 0xFFFF) | ((pkt.arrival_time & 0xFFFF) << 16);
          dmem[base_addr + 4] = pkt.payload_ptr;
        }

        dmem_in_t dmem_din = wb2dmem_ch.Pop();  // Blocking pop
        unsigned int addr = dmem_din.data_addr;

        dmem_out_t dmem_dout;
        if (dmem_din.read_en) {
          dmem_dout.data_out = dmem[addr];
          dmem2wb_ch.Push(dmem_dout);
        } else if (dmem_din.write_en) {
          dmem[addr] = dmem_din.data_in;
          dmem_dout.data_out = dmem_din.data_in;
        }
      }
      wait();
    }
  }

  // Set coordinates (x, y) for the node's parent (if any)
  void set_parent(sc_uint<4> x, sc_uint<4> y) {
    parent_id[0] = x;
    parent_id[1] = y;
  }

#ifndef __SYNTHESIS__
  void dump_memory() const {
    std::cout << "[SchedulingNode] Dumping internal memory:\n";
    for (unsigned i = 0; i < MEM_SIZE; ++i) {
      const packet_metadata_t& pkt = memory[i];
      std::cout << "  [" << i << "] "
                << "src=" << pkt.src << ", dst=" << pkt.dst
                << ", length=" << pkt.length << ", payload_ptr=0x" << std::hex
                << pkt.payload_ptr << std::dec << "\n";
    }
  }
  void dump_dmem(unsigned count = 16) const {
    std::cout << "[SchedulingNode] Dumping DMEM:" << std::endl;
    for (unsigned i = 0; i < count; ++i) {
      std::cout << "  dmem[" << i << "] = 0x" << std::hex << dmem[i] << std::dec
                << std::endl;
    }
  }
#endif
};

#endif  // __NODE__H
