// Based on M. Elbediwy's work
// Enabling Rank-Based P4 Programmable Schedulers: Requirements, Implementation,
// and Evaluation on BMv2 Switches

#define META_ADDR ((volatile unsigned int*)0x100)
#define DMEM_BASE ((volatile unsigned int*)0x150)
#define WEIGHT_TABLE ((volatile unsigned int*)0x180)      // Quantum per flow
#define SRV_CNTR_BASE \
  ((volatile unsigned int*)0x1D0)  // Service counter per flow
#define DEQ_CYCLE_PTR ((volatile unsigned int*)0x210)  // Global dequeue cycle

#define N 8              // Number of flows
#define MIN_PKT_SIZE 64  // Smallest allowed packet length (in bytes)

void notmain() {
  unsigned int flow_id = META_ADDR[3] & 0xFFFF;
  unsigned int pkt_len = META_ADDR[2] & 0xFFFF;
  unsigned int Q = WEIGHT_TABLE[flow_id];  // flow's quantum
  unsigned int deq_cycle = DEQ_CYCLE_PTR[0];
  unsigned int pkts_per_rnd = N * (Q / MIN_PKT_SIZE);

  // Step 3: max(srv_cntr[flow_id], deq_cycle * Q)
  unsigned int srv_cntr = SRV_CNTR_BASE[flow_id];
  if (srv_cntr < (deq_cycle * Q)) {
    srv_cntr = deq_cycle * Q;
  }

  // Step 4: add packet length
  srv_cntr += pkt_len;
  SRV_CNTR_BASE[flow_id] = srv_cntr;

  // Step 5: virtual round ID
  unsigned int virtual_round_id = (srv_cntr - 1) / Q;

  // Step 6: compute rank
  unsigned int rank = flow_id + (pkts_per_rnd * virtual_round_id);

  // Output rank and round_id to DMEM
  DMEM_BASE[0] = rank;
  DMEM_BASE[1] = virtual_round_id;

  // Step 8: update global dequeue cycle
  if (deq_cycle < virtual_round_id) {
    DEQ_CYCLE_PTR[0] = virtual_round_id;
  }
}
