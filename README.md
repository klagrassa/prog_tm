# A P4 Traffic Manager Prototyping Platform

This workspace provides a platform for simulating and evaluating P4-based traffic management and scheduling policies. It combines a BMv2 prototype (software switch) and a SystemC (HLS compatible) description of a scheduling node.

---

## Project Structure

- [`prog_tm_bmv2/`](prog_tm_bmv2/README.md):  
  Contains the BMv2 prototype, a reference P4 software switch.  
  - Implements the [BMv2 Simple Switch target](prog_tm_bmv2/docs/simple_switch.md)
  - Includes scripts for building, testing, and running P4 programs.
  - Integrates with Mininet for network emulation.
  - See [BMv2 README](prog_tm_bmv2/README.md) for installation and usage.

- [`scheduling_node_hls/`](scheduling_node_hls/):  
  Contains a SystemC description of a scheduling node for hardware-level simulation and synthesis.
  - Used for high-level modeling and validation of scheduling algorithms.
  - Can be integrated with the BMv2 prototype for co-simulation.

---

## How to Build

1. **Configure the BMv2 prototype:**
   ```sh
   cd prog_tm_bmv2
   ./autogen.sh
   ./configure --enable-tm-debug --with-pi --with-thrift --enable-debugger 'CXXFLAGS=-O0 -g' --with-pdfixed --enable-Werror
   make
   sudo make install
   ```

2. **Build the SystemC scheduling node:**
   - Follow instructions in [`scheduling_node_hls/`](scheduling_node_hls/) (see its README or build scripts).
   - It builds the DRIM CPU and generates the `sim_sc` executable, which simulates the CPU using a compiled program based on templates in `core/src/schedulers`

3. **Node synthesis via HLS**
   - Requires [Catapult HLS](https://www.mentor.com/hls-lp/catapult-high-level-synthesis/) for hardware synthesis (tested on Catapult HLS 2025.1).
   - To synthesize the node, navigate to the `scheduling_node_hls/core` directory and run:
     ```sh
     catapult -f hls_to_synth_node_only.tcl
     ```

---

## How to Run

- **Compile your P4 program:**  
  Use the provided P4 compiler in [`prog_tm_bmv2/papers_results/`](prog_tm_bmv2/papers_results/README.md):
  ```sh
  ./p4c-bm2-ss forwarding.p4 --emit-externs
  ```
- **Configure the network topology:**  
  Use Mininet script [`prog_tm_bmv2/papers_results/mininet_env.py`](prog_tm_bmv2/papers_results/mininet_env.py).

- **Load switch CLI commands:**  
  Use CLI script [`prog_tm_bmv2/papers_results/s1-commands.txt`](prog_tm_bmv2/papers_results/s1-commands.txt/).

- **Generate and send test packets:**  
  Use Scapy scripts in [`prog_tm_bmv2/papers_results/pkt_gen.py`](prog_tm_bmv2/papers_results/pkt_gen.py).

---

## Logging and Debugging

- **Traffic Manager and Node logs:**  
  Output CSV files such as `packet_log_in0.csv` and `packet_log_out0.csv` are generated for packet-level analysis.
- **BMv2 event logging:**  
  Enable with the `--nanolog` option (see [BMv2 README](prog_tm_bmv2/README.md#displaying-the-event-logging-messages)).
- **Debugger:**  
  Enable with `--enable-debugger` during configuration and use [tools/p4dbg.py](prog_tm_bmv2/tools/p4dbg.py).

---

## Extending and Customizing

- **Add new scheduling policies:**  
  See [How to add policies](prog_tm_bmv2/papers_results/README.md#how-to-add-policies).
- **Integrate custom externs:**  
  See [Custom Extern Example](prog_tm_bmv2/examples/custom_extern/README.md).

---

## References

- [BMv2 Documentation](prog_tm_bmv2/docs/simple_switch.md)
- [P4 Language Specification](https://p4.org/specs/)
- [Mininet](http://mininet.org/)
- [SystemC](https://www.accellera.org/community/systemc)

---

## License

See [prog_tm_bmv2/LICENSE](prog_tm_bmv2/LICENSE).# prog_tm
