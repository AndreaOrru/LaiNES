#include <cstdio>
#include "cartridge.hpp"

namespace Cartridge {


int banksMap[8];  // Map virtual memory to ROM.
u8* ROM;

/* Cartridge access */
template <bool wr> u8 access(u16 addr, u8 v)
{
    addr -= 0x8000;
    return ROM[banksMap[addr / 0x1000] + (addr % 0x1000)];
}
template u8 access<0>(u16, u8); template u8 access<1>(u16, u8);

/* Load ROM from file */
void load(const char* fname)
{
    FILE* f = fopen(fname, "rb");
    
    // Extract info from iNES header:
    u8 header[16]; fread(header, 16, 1, f);
    u8 prgROM_16k = header[4];

    // Read the PRG-ROM:
    ROM = new u8[0x4000 * prgROM_16k];
    fread(ROM, 0x4000, prgROM_16k, f);
    fclose(f);

    // Initialize the mapping:
    for (int i = 0; i < 8; i++)
        banksMap[i] = (i % (prgROM_16k*4)) * 0x1000;
}


}
