#pragma once
#include "mapper.hpp"


class Mapper2 : public Mapper
{
    u8 regs[1];
    bool vertical_mirroring;

    void apply();

  public:
    Mapper2(u8* rom) : Mapper(rom)
    {
        regs[0] = 0;
        vertical_mirroring = rom[6] & 0x01;
        apply();
    }

    u8 write(u16 addr, u8 v);
    u8 chr_write(u16 addr, u8 v);
};
