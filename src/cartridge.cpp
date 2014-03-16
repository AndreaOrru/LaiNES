#include <cstdio>
#include "mappers/mapper0.hpp"
#include "cartridge.hpp"

namespace Cartridge {


Mapper* mapper;

/* PRG-ROM access */
template <bool wr> u8 access(u16 addr, u8 v)
{
    if (!wr) return mapper->read(addr);
    else            mapper->write(addr, v);
    return v;
}
template u8 access<0>(u16, u8); template u8 access<1>(u16, u8);

/* CHR-ROM/RAM access */
template <bool wr> u8 chr_access(u16 addr, u8 v)
{
    if (!wr) return mapper->chr_read(addr);
    else            mapper->chr_write(addr, v);
    return v;
}
template u8 chr_access<0>(u16, u8); template u8 chr_access<1>(u16, u8);

void load(const char* fileName)
{
    FILE* f = fopen(fileName, "rb");

    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);

    u8* rom = new u8[size];
    fread(rom, size, 1, f);
    fclose(f);

    int mapperNum = (rom[7] & 0xF0) | (rom[6] >> 4);
    switch (mapperNum)
    {
        case 0:  mapper = new Mapper0(rom);
    }
}


}
