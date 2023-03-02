#ifndef REG_BITS_H
#define REG_BITS_H
#include <registers.h>


extern RegMemory reg_mem;

struct operation_ctrl_bits
{
    unsigned OP_MODE    : 2;
    unsigned            : 1;
    unsigned DUMP       : 1;
    unsigned MUTE_RPL   : 1;
    unsigned VISUALEN   : 1;
    unsigned OPLEDEN    : 1;
    unsigned ALIVE_EN   : 1;
};


struct reset_dev_bits
{
    unsigned reset_def  : 1;
    unsigned reset_ee   : 1;
    unsigned save       : 1;
    unsigned            : 3;
    unsigned boot_def   : 1;
    unsigned boot_ee    : 1;
};

struct clock_config_bits
{
    unsigned clk_rep    : 1;
    unsigned clk_gen    : 1;
    unsigned            : 1;
    unsigned rep_able   : 1;
    unsigned gen_able   : 1;
    unsigned            : 1;
    unsigned clk_unlock : 1;
    unsigned clk_lock   : 1;
};



#endif // REG_BITS_H
