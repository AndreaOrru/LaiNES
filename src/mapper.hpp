#pragma once
#include <cstring>
#include "common.hpp"


class Mapper
{
  protected:
    int banksMap[8];
    u8 *rom, *vRam;
    u32 prgSize, chrSize;

  public:
    Mapper(u8* rom)
    {
        prgSize = rom[4] * 0x4000;
        chrSize = rom[5] * 0x2000;

        this->rom  = new u8[prgSize];
        this->vRam = new u8[chrSize];

        memcpy(this->rom , 16 + rom          , prgSize);
        memcpy(this->vRam, 16 + rom + prgSize, chrSize);
    }

    u8 read(u16 addr)
    {
        addr -= 0x8000;
        return rom[banksMap[addr / 0x1000] + (addr % 0x1000)];
    }
    virtual void write(u16 addr, u8 v) {};

    virtual u8 chr_read(u16 addr)
    {
        return vRam[addr];
    }
    virtual void chr_write(u16 addr, u8 v) {};
};
