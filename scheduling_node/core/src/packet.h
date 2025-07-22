#ifndef PACKET_H_
#define PACKET_H_

#include <ac_int.h>
#include <mc_connections.h>

// Metadata structure for packets descriptors in the traffic manager.
// Total size is 155 bits
struct packet_metadata_t {
  // Standard routing fields
  sc_uint<32> src;
  sc_uint<32> dst;

  // Payload characteristics
  sc_uint<16> length;   // Packet length in bytes
  sc_uint<8> tos;       // Type of Service (can include DSCP, ECN)
  sc_uint<3> priority;  // Explicit priority (0 = low, 7 = high)

  // Scheduling-specific
  sc_uint<16> flow_id;       // To associate packets from same flow
  sc_uint<16> arrival_time;  // Timestamp (could be cycle or abstract time)

  // Pointer to actual payload (index in device's main memory)
  sc_uint<32> payload_ptr;

  // NEEDED BY CATAPULT HLS !
  static const unsigned int width = 32 + 32 + 16 + 8 + 3 + 16 + 16 + 32;

  // For Connections marshalling
  template <unsigned int Size>
  void Marshall(Marshaller<Size>& m) {
    m & src;
    m & dst;
    m & length;
    m & tos;
    m & priority;
    m & flow_id;
    m & arrival_time;
    m & payload_ptr;
  }

  bool operator==(const packet_metadata_t& rhs) const {
    return src          == rhs.src &&
           dst          == rhs.dst &&
           length       == rhs.length &&
           tos          == rhs.tos &&
           priority     == rhs.priority &&
           flow_id      == rhs.flow_id &&
           arrival_time == rhs.arrival_time &&
           payload_ptr  == rhs.payload_ptr;
  }
};

// For SystemC tracing
inline void sc_trace(sc_trace_file* tf, const packet_metadata_t& pkt,
                     const std::string& name) {
  sc_trace(tf, pkt.src, name + ".src");
  sc_trace(tf, pkt.dst, name + ".dst");
  sc_trace(tf, pkt.length, name + ".length");
  sc_trace(tf, pkt.tos, name + ".tos");
  sc_trace(tf, pkt.priority, name + ".priority");
  sc_trace(tf, pkt.flow_id, name + ".flow_id");
  sc_trace(tf, pkt.arrival_time, name + ".arrival_time");
  sc_trace(tf, pkt.payload_ptr, name + ".payload_ptr");
}

// Stream operator for printing
inline std::ostream& operator<<(std::ostream& os,
                                const packet_metadata_t& pkt) {
  os << "("
     << "src=" << pkt.src << ", dst=" << pkt.dst << ", length=" << pkt.length
     << ", tos=" << pkt.tos << ", priority=" << pkt.priority
     << ", flow_id=" << pkt.flow_id << ", arrival_time=" << pkt.arrival_time
     << ", payload_ptr=0x" << std::hex << pkt.payload_ptr << std::dec << ")";
  return os;
}

// IMEM runtime data
struct imem_write_req_t {
  sc_uint<XLEN> addr;  // Word-aligned address
  sc_uint<XLEN> data;

  static const unsigned int width = XLEN + XLEN;

  // Default constructor
  imem_write_req_t() : addr(0), data(0) {}

  // For Connections marshalling
  template <unsigned int Size>
  void Marshall(Marshaller<Size>& m) {
    m & addr;
    m & data;
  }

  bool operator==(const imem_write_req_t& rhs) const {
    return addr == rhs.addr && data == rhs.data;
  }
} ;

// For SystemC tracing
inline void sc_trace(sc_trace_file* tf, const imem_write_req_t& req, const std::string& name) {
  sc_trace(tf, req.addr, name + ".addr");
  sc_trace(tf, req.data, name + ".data");
}

// Stream operator for printing
inline std::ostream& operator<<(std::ostream& os, const imem_write_req_t& req) {
  os << "(addr=0x" << std::hex << req.addr << ", data=0x" << req.data << std::dec << ")";
  return os;
}

#endif  // PACKET_H_
