options set Input/CppStandard c++11
set_working_dir .
solution file add ./src/node.h
go compile
solution library add mgc_Xilinx-ARTIX-7-3_beh -- -rtlsyntool Vivado -manufacturer Xilinx -family ARTIX-7 -speed -3 -part xc7a200tsbg484-3
solution library add Xilinx_RAMS
go libraries
directive set -CLOCKS {clk {-CLOCK_PERIOD 10 -CLOCK_HIGH_TIME 5 -CLOCK_OFFSET 0.000000 -CLOCK_UNCERTAINTY 0.0}}
go assembly
go architect
go allocate
go extract
