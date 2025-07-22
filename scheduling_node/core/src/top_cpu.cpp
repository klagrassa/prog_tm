#include <iostream>

#include "drim4hls_datatypes.h"
#include "defines.h"
#include "globals.h"
#include "drim4hls.h"

#include <mc_scverify.h>
#include <ac_int.h>

class Top: public sc_module {
    public:

    CCS_DESIGN(drim4hls) CCS_INIT_S1(m_dut);

    sc_clock clk;
    SC_SIG(bool, rst);

    // End of simulation signal.
    #pragma hls_direct_input
    sc_signal < bool > CCS_INIT_S1(program_end);

    // Instruction counters
    #pragma hls_direct_input
    sc_signal < long int > CCS_INIT_S1(icount);
    #pragma hls_direct_input
    sc_signal < long int > CCS_INIT_S1(j_icount);
    #pragma hls_direct_input
    sc_signal < long int > CCS_INIT_S1(b_icount);
    #pragma hls_direct_input
    sc_signal < long int > CCS_INIT_S1(m_icount);
    #pragma hls_direct_input
    sc_signal < long int > CCS_INIT_S1(o_icount);

    /* The testbench, DUT, IMEM and DMEM modules. */
    Connections::Combinational < imem_out_t > CCS_INIT_S1(imem2de_ch);
    Connections::Combinational < imem_in_t > CCS_INIT_S1(fe2imem_ch);

    Connections::Combinational < dmem_out_t > CCS_INIT_S1(dmem2wb_ch);
    Connections::Combinational < dmem_in_t > CCS_INIT_S1(wb2dmem_ch);

    sc_uint < XLEN > imem[ICACHE_SIZE];

    imem_out_t imem_dout;
    imem_in_t imem_din;

    sc_uint < XLEN > dmem[DCACHE_SIZE];

    dmem_out_t dmem_dout;
    dmem_in_t dmem_din;

    unsigned long long cycle_count;
    const std::string testing_program;
    
    int wait_stalls;

    SC_CTOR(Top);
    Top(const sc_module_name &name, const std::string &testing_program): 
    clk("clk", 10, SC_NS, 5, 0, SC_NS, true),
    m_dut("drim4hls"),
    testing_program(testing_program) {
        
        Connections::set_sim_clk( & clk);

        // Connect the design module
        m_dut.clk(clk);
        m_dut.rst(rst);
        m_dut.program_end(program_end);

        m_dut.icount(icount);
        m_dut.j_icount(j_icount);
        m_dut.b_icount(b_icount);
        m_dut.m_icount(m_icount);
        m_dut.o_icount(o_icount);

        m_dut.imem2de_data(imem2de_ch);
        m_dut.fe2imem_data(fe2imem_ch);
        m_dut.dmem2wb_data(dmem2wb_ch);
        m_dut.wb2dmem_data(wb2dmem_ch);

        SC_CTHREAD(run, clk);

        SC_THREAD(imemory_th);
        sensitive << clk.posedge_event();
        async_reset_signal_is(rst, false);

        SC_THREAD(dmemory_th);
        sensitive << clk.posedge_event();
        async_reset_signal_is(rst, false);
    }

    void imemory_th() {
        IMEM_RST: {
            imem2de_ch.ResetWrite();
            fe2imem_ch.ResetRead();

            wait();
        }
        IMEM_BODY: while (true) {
            imem_din = fe2imem_ch.Pop();

            unsigned int addr_aligned = imem_din.instr_addr >> 2;
			//std::cout << "imem addr= " << addr_aligned << endl;
            
            imem_dout.instr_data = imem[addr_aligned];
			
            // unsigned int random_stalls = (rand() % 2) + 1;
            //unsigned int random_stalls = 1;
            wait(1);

            imem2de_ch.Push(imem_dout);
            wait();
        }

    }

    void dmemory_th() {
        DMEM_RST: {
            wb2dmem_ch.ResetRead();
            dmem2wb_ch.ResetWrite();
			wait_stalls = 0;
            wait();
        }
        DMEM_BODY: while (true) {
            dmem_din = wb2dmem_ch.Pop();
            unsigned int addr = dmem_din.data_addr;
			//std::cout << "dmem addr= " << addr << endl;
            // unsigned int random_stalls = (rand() % 25) + 1;
            
            // //std::cout << "wait=" << random_stalls << endl;
            // //unsigned int random_stalls = 15;
            // wait_stalls += random_stalls;
            // wait(random_stalls);
            wait(1);
            // std::cout << "wait= " << random_stalls << endl;
            std::cout << "wait= 1" << endl;
            
            if (dmem_din.read_en) {
				std::cout << "dmem read" << endl;
                dmem_dout.data_out = dmem[addr];
                dmem2wb_ch.Push(dmem_dout);
            } else if (dmem_din.write_en) {
				std::cout << "dmem write" << endl;
                dmem[addr] = dmem_din.data_in;
                dmem_dout.data_out = dmem_din.data_in;
            }

            // REMOVE
            std::cout << "dmem[" << addr << "]=" << dmem[addr] << endl;
            wait();
        }

    }

    // Scheduling node add
    void inject_packet_metadata(unsigned addr, sc_uint<XLEN> value) {
        if (addr < DCACHE_SIZE) {
        dmem[addr] = value;
        }
    }

    void run() {

        std::ifstream load_program;
        load_program.open(testing_program, std::ifstream:: in );
        unsigned index;
        unsigned address;
        unsigned data;
        
        while (load_program >> std::hex >> address) {

            index = address >> 2;
            if (index >= ICACHE_SIZE) {
                SC_REPORT_ERROR(sc_object::name(), "Program larger than memory size.");
                sc_stop();
                return;
            }
            load_program >> data;
            imem[index] = (ac_int<32, false>) data;
            std::cout << "imem[" << index << "]=" << imem[index] << endl;
            dmem[index] = imem[index];
        }

        load_program.close();

        rst.write(0);
        wait(5);
        rst.write(1);
        wait();

        // Packet injection
        unsigned base_addr = 0x100 >> 2;
        unsigned weight_addr = 0x180 >> 2;
        unsigned deq_cycle_addr = 0x210 >> 2;
        sc_uint<16> flow_id = 0x01;
        sc_uint<32> quantum = 128;      // Example quantum value
        sc_uint<32> deq_cycle = 0x10;   // Example dequeue cycle value

        // Example packet fields
        sc_uint<32> src         = 0x01;
        sc_uint<32> dst         = 0x02;
        sc_uint<16> length      = 64;
        sc_uint<8>  tos         = 0x1;
        sc_uint<3>  priority    = 5;
        sc_uint<16> arrival     = 0x10;
        sc_uint<32> payload_ptr = 0xDEADBEEF;

        // Inject packet metadata
        inject_packet_metadata(base_addr + 0, src);
        inject_packet_metadata(base_addr + 1, dst);
        inject_packet_metadata(base_addr + 2, (length & 0xFFFF) | ((tos & 0xFF) << 16) | ((priority & 0x7) << 24));
        inject_packet_metadata(base_addr + 3, (flow_id & 0xFFFF) | ((arrival & 0xFFFF) << 16));
        inject_packet_metadata(base_addr + 4, payload_ptr);

        // Inject quantum (weight) for the flow
        inject_packet_metadata(weight_addr + flow_id, quantum);

        // Inject dequeue cycle
        inject_packet_metadata(deq_cycle_addr, deq_cycle);

        cycle_count = 0;
        do {
            wait();
            cycle_count++;
        } while (!program_end.read());
        wait(5);
        // cycle_count += 5; // Final 5 cycles
        
        sc_stop();
        int dmem_index;
        for (dmem_index = 0; dmem_index < 400; dmem_index++) {
            std::cout << "dmem[" << dmem_index << "]=" << dmem[dmem_index] << endl;
        }
        std::cout << "wait_stalls " << wait_stalls << endl;

        long icount_end, j_icount_end, b_icount_end, m_icount_end, o_icount_end, pre_b_icount_end;

        icount_end = icount.read();
        j_icount_end = j_icount.read();
        b_icount_end = b_icount.read();
        m_icount_end = m_icount.read();
        o_icount_end = o_icount.read();

        SC_REPORT_INFO(sc_object::name(), "Program complete.");

        std::cout << "INSTR TOT: " << icount_end << std::endl;
        std::cout << "   JUMP  : " << j_icount_end << std::endl;
        std::cout << "   BRANCH: " << b_icount_end << std::endl;
        std::cout << "   MEM   : " << m_icount_end << std::endl;
        std::cout << "   OTHER : " << o_icount_end << std::endl;
        std::cout << "   CYCLES COUNT: " << cycle_count << std::endl;

    }

};

int sc_main(int argc, char * argv[]) {

    if (argc == 1) {
        std::cerr << "Usage: " << argv[0] << " <testing_program>" << std::endl;
        std::cerr << "where:  <testing_program> - path to .txt file of the testing program" << std::endl;
        return -1;
    }

    std::string testing_program = argv[1];

    Top top("top", testing_program);
    sc_start();
    return 0;
}
