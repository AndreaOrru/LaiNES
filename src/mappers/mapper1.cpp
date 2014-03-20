#include "ppu.hpp"
#include "mappers/mapper1.hpp"


/* Apply the registers state */
void Mapper1::apply()
{
    // 16KB PRG:
    if (regs[0] & 0b1000)
    {
        // 0x8000 swappable, 0xC000 fixed to bank 0x0F:
        if (regs[0] & 0b100)
        {
            map_prg<16>(0, regs[3] & 0xF);
            map_prg<16>(1,           0xF);
        }
        // 0x8000 fixed to bank 0x00, 0xC000 swappable:
        else
        {
            map_prg<16>(0, 0);
            map_prg<16>(1, regs[3] & 0xF);
        }
    }
    // 32KB PRG:
    else
        map_prg<32>(0, (regs[3] & 0xF) >> 1);

    // 4KB CHR:
    if (regs[0] & 0b10000)
    {
        map_chr<4>(0, regs[1]);
        map_chr<4>(1, regs[2]);
    }
    // 8KB CHR:
    else
        map_chr<8>(0, regs[1] >> 1);

    // Set mirroring:
    switch (regs[0] & 0b11)
    {
        case 2:  set_mirroring(PPU::VERTICAL);   break;
        case 3:  set_mirroring(PPU::HORIZONTAL); break;
    }
}

u8 Mapper1::write(u16 addr, u8 v)
{
    // PRG RAM write;
    if (addr < 0x8000)
        prgRam[addr - 0x6000] = v;
    // Mapper register write:
    else if (addr & 0x8000)
    {
        // Reset:
        if (v & 0x80)
        {
            writeN   = 0;
            tmpReg   = 0;
            regs[0] |= 0x0C;
            apply();
        }
        else
        {
            // Write a bit into the temporary register:
            tmpReg = ((v & 1) << 4) | (tmpReg >> 1);
            // Finished writing all the bits:
            if (++writeN == 5)
            {
                regs[(addr >> 13) & 0b11] = tmpReg;
                writeN = 0;
                tmpReg = 0;
                apply();
            }
        }
    }
    return v;
}

u8 Mapper1::chr_write(u16 addr, u8 v)
{
    return chr[addr] = v;
}
