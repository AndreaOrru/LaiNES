#pragma once
#include "mapper.hpp"


class Mapper3 : public Mapper
{
    u8 regs[1];
    bool vertical_mirroring;
    bool PRG_size_16k;
    void apply();

    public:
    Mapper3(u8* rom) : Mapper(rom)
    {
        PRG_size_16k = rom[4] == 1;
        vertical_mirroring = rom[6] & 0x01;
        regs[0] = 0;
        apply();
    }

    u8 write(u16 addr, u8 v);
    u8 chr_write(u16 addr, u8 v);
};

