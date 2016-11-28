#include "ppu.hpp"
#include "mappers/mapper3.hpp"

/* Based off of https://wiki.nesdev.com/w/index.php/INES_Mapper_003 */

/* Apply the registers state */
void Mapper3::apply()
{
    if (PRG_size_16k)
    {
    /*
     * mirror the bottom on the top
     * 0x8000 - 0xBFFF ==
     * 0xC000 - 0xFFFF
     */
        map_prg<16>(0,0);
        map_prg<16>(1,0);
    }
    else
    {
        /* no mirroring */
        map_prg<16>(0,0);
        map_prg<16>(1,1);
    }

    /* 8k bankswitched CHR */
    map_chr<8>(0, regs[0] & 0b11);

    /* mirroring is based on the header (soldered) */
    set_mirroring(vertical_mirroring?PPU::VERTICAL:PPU::HORIZONTAL);
}

u8 Mapper3::write(u16 addr, u8 v)
{
    /* check for bus contingency? */

    /* chr bank switching */
    if (addr & 0x8000)
    {
      regs[0] = v;
      apply();
    }
    return v;
}

u8 Mapper3::chr_write(u16 addr, u8 v)
{
    return chr[addr] = v;
}

