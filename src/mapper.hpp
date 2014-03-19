#pragma once
#include <cstring>
#include "common.hpp"


class Mapper
{
  protected:
    u32 prgMap[4];
    u32 chrMap[8];

    u8 *prg, *chr, *prgRam;
    u32 prgSize, chrSize, prgRamSize;

    void map_prg32k(int bank);
    void map_prg16k(int slot, int bank);
    void map_chr8k(int bank);
    void map_chr4k(int slot, int bank);

  public:
    Mapper(u8* rom);

    u8 read(u16 addr);
    virtual u8 write(u16 addr, u8 v) { return v; };

    u8 chr_read(u16 addr);
    virtual u8 chr_write(u16 addr, u8 v) { return v; };
};
