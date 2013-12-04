#include "common.hpp"

#ifndef CPU_HPP
#define CPU_HPP
namespace CPU {


enum IntType { NMI, RESET, IRQ, BRK };  // Interrupt type.
typedef u16 (*Mode)(void);              // Addressing mode.

/* Processor flags */
enum Flag {C, Z, I, D, B, UNUSED, V, N};
union Flags
{
    struct
    {
        bool c : 1;      // Carry.
        bool z : 1;      // Zero.
        bool i : 1;      // Interrupt priority.
        bool d : 1;      // Decimal (unused).
        bool b : 1;
        bool unused : 1;
        bool v : 1;      // Overflow.
        bool n : 1;      // Negative.
    };
    u8 reg;

    bool get(Flag i)         { return reg & (1 << i); }
    void set(Flag i, bool v) { reg = v ? (reg | (1 << i)) : (reg & ~(1 << i)); }
};

void set_nmi();
void power();
void run();


}
#endif  // CPU_HPP
