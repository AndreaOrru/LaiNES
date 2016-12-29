#pragma once
#include "mapper.hpp"
#include "ppu.hpp"

class Mapper7 : public Mapper
{
    u8 regs[1];
    void apply();

  public:
    Mapper7(u8* rom) : Mapper(rom)
    {
        regs[0] = 0;
        apply();
    }

    u8 write(u16 addr, u8 v);
    u8 chr_write(u16 addr, u8 v);
};
