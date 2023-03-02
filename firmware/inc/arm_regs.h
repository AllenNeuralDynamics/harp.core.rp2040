#ifndef ARM_REGS_H
#define ARM_REGS_H



// Note: PPB_BASE defines the start of the Cortex M0+ internal peripherals.
//  It is defined elsewhere.

// More info on SYSTICK.
// https://developer.arm.com/documentation/dui0552/a/cortex-m3-peripherals/system-timer--systick?lang=en

// Bitfield implementation for easy access to register fields.
// SYSTICK
struct syst_csrbits
{
    unsigned ENABLE     : 1;
    unsigned TICKINT    : 1;
    unsigned CLKSOURCE  : 1;
    unsigned            : 12;
    unsigned COUNTFLAG  : 1;
    unsigned            : 16;
};
#define SYST_CSRbits (*(volatile syst_csrbits*) (PPB_BASE + 0xe010))
#define SYST_CSR (*(volatile uint32_t*)(PPB_BASE + 0xe010))

struct syst_rvrbits
{
    unsigned RELOAD : 24;
    unsigned        : 8;
};
#define SYST_RVRbits (*(volatile syst_rvrbits*) (PPB_BASE + 0xe014))
#define SYST_RVR (*(volatile uint32_t*)(PPB_BASE + 0xe014))


struct syst_cvrbits
{
    unsigned CURRENT    : 24;
    unsigned            : 8;
};
#define SYST_CVRbits (*(volatile syst_cvrbits*) (PPB_BASE + 0xe018))
#define SYST_CVR (*(volatile uint32_t*)(PPB_BASE + 0xe018))

#endif // ARM_REGS_H
