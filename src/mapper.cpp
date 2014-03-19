#include "mapper.hpp"

Mapper::Mapper(u8* rom)
{
    // Read infos from header:
    prgSize    = rom[4] * 0x4000;
    chrSize    = rom[5] * 0x2000;
    prgRamSize = rom[8] ? rom[8] * 0x2000 : 0x2000;

    this->prg    = rom + 16;
    this->prgRam = new u8[prgRamSize];

    // CHR ROM:
    if (chrSize)
        this->chr = rom + 16 + prgSize;
    // CHR RAM:
    else
    {
        chrSize = 0x2000;
        this->chr = new u8[chrSize];
    }
}

/* Access to memory */
u8 Mapper::read(u16 addr)
{
    if (addr >= 0x8000)
        return prg[prgMap[(addr - 0x8000) / 0x2000] + ((addr - 0x8000) % 0x2000)];
    else
        return prgRam[addr - 0x6000];
}

u8 Mapper::chr_read(u16 addr)
{
    return chr[chrMap[addr / 0x400] + (addr % 0x400)];
}

/* PRG mapping functions */
void Mapper::map_prg32k(int bank)
{
    for (int i = 0; i < 4; i++)
        prgMap[i] = (0x8000*bank + 0x2000*i) % prgSize;
}

void Mapper::map_prg16k(int slot, int bank)
{
    for (int i = 0; i < 2; i++)
        prgMap[2*slot + i] = (0x4000*bank + 0x2000*i) % prgSize;
}

/* CHR mapping functions */
void Mapper::map_chr8k(int bank)
{
    for (int i = 0; i < 8; i++)
        chrMap[i] = (0x2000*bank + 0x400*i) % chrSize;
}

void Mapper::map_chr4k(int slot, int bank)
{
    for (int i = 0; i < 4; i++)
        chrMap[4*slot + i] = (0x1000*bank + 0x400*i) % chrSize;
}
