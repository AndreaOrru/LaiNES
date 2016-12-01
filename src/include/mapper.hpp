#pragma once
#include <cstring>
#include "common.hpp"


class Mapper
{
    u8* rom;
    bool chrRam = false;

  protected:
    u32 prgMap[4];
    u32 chrMap[8];

    u8 *prg, *chr, *prgRam;
    u32 prgSize, chrSize, prgRamSize;

    template <int pageKBs> void map_prg(int slot, int bank);
    template <int pageKBs> void map_chr(int slot, int bank);

  public:
    Mapper(u8* rom);
    ~Mapper();

    u8 read(u16 addr);
    virtual u8 write(u16 addr, u8 v) { return v; }

    u8 chr_read(u16 addr);
    virtual u8 chr_write(u16 addr, u8 v) { return v; }

    virtual void signal_scanline() {}
};
