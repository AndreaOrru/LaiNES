#include "types.hpp"

#ifndef CPU_HPP
#define CPU_HPP


/* Processor flags */
enum {C, Z, I, D, B, UNUSED, V, N};
union Flags
{
    struct
    {
        bool c : 1;
        bool z : 1;
        bool i : 1;
        bool d : 1;
        bool b : 1;
        bool unused : 1;
        bool v : 1;
        bool n : 1;
    };
    u8 reg;

    bool get(u8 i) { return reg & (1 << i); }
    void set(u8 i, bool v) { reg = v ? (reg | (1 << i)) : (reg & ~(1 << i)); }
};

// Addressing mode:
typedef u16 (*Mode)(void);


#endif  // CPU_HPP
