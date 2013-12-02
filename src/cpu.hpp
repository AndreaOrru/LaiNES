#include "types.hpp"

#ifndef CPU_HPP
#define CPU_HPP
namespace CPU {


// Addressing mode:
typedef u16 (*Mode)(void);

/* Processor flags */
enum {C, Z, I, D, B, UNUSED, V, N};
union Flags
{
    struct
    {
        bool c : 1;      // Carry.
        bool z : 1;      // Zero.
        bool i : 1;      // Interrupt priority.
        bool d : 1;      // Decimal (useless).
        bool b : 1;
        bool unused : 1;
        bool v : 1;      // Overflow.
        bool n : 1;      // Negative.
    };
    u8 reg;

    bool get(u8 i) { return reg & (1 << i); }
    void set(u8 i, bool v) { reg = v ? (reg | (1 << i)) : (reg & ~(1 << i)); }
};

void set_nmi();


}
#endif  // CPU_HPP
