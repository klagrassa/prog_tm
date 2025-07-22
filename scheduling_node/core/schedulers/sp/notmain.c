#define DMEM_BASE            ((volatile unsigned int*)0x150)
#define PACKET_METADATA_ADDR ((volatile unsigned int*)0x100)
#define DMEM_PACKET_VALID    ((volatile unsigned int*)0x200)
#define DMEM_PROGRAM_END     ((volatile unsigned int*)0x204)

void notmain() {
    // Wait for packet_valid
    // while (*DMEM_PACKET_VALID == 0);

    // Process packet: extract priority and write rank
    unsigned int metadata_word2 = PACKET_METADATA_ADDR[2];
    unsigned int priority = (metadata_word2 >> 24) & 0x7;
    DMEM_BASE[0] = priority; // Write rank

    // Signal done
    // *DMEM_PROGRAM_END = 1;

    // // Wait for packet_valid to be cleared
    // while (*DMEM_PACKET_VALID != 0);

    // // Clear program_end for next round
    // *DMEM_PROGRAM_END = 0;
}