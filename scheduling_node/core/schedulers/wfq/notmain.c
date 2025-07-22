#define DMEM_BASE        ((volatile unsigned int*)0x150)
#define META_ADDR        ((volatile unsigned int*)0x100)
#define FINISH_TIME_BASE ((volatile unsigned int*)0x80)
#define WEIGHT_TABLE     ((volatile unsigned int*)0x180)
#define VIRTUAL_TIME_PTR ((volatile unsigned int*)0x208)

void notmain() {
    unsigned int flow_id     = META_ADDR[3] & 0xFFFF;
    unsigned int packet_len  = META_ADDR[2] & 0xFFFF;
    unsigned int flow_weight = WEIGHT_TABLE[flow_id];
    unsigned int last_finish = FINISH_TIME_BASE[flow_id];
    unsigned int virtual_time = VIRTUAL_TIME_PTR[0];

    unsigned int start_time  = (last_finish > virtual_time) ? last_finish : virtual_time;
    unsigned int finish_time = start_time + (packet_len / flow_weight);

    FINISH_TIME_BASE[flow_id] = finish_time;
    VIRTUAL_TIME_PTR[0]       = finish_time;
    DMEM_BASE[0]              = finish_time;
}
