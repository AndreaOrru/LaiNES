#pragma once
#include "mapper.hpp"


class Mapper1 : public Mapper
{
  private:
    int writeN;
    u8 tmpReg;
    u8 regs[4];

    void apply()
    {
        // Mirroring.

        // 16KB PRG:
        if (regs[0] & 0b1000)
        {
            // 0x8000 swappable, 0xC000 fixed to bank 0x0F:
            if (regs[0] & 0b100)
            {
                map_prg16k(0, regs[3] & 0xF);
                map_prg16k(1, 0xF);
            }
            // 0x8000 fixed to bank 0x00, 0xC000 swappable:
            else
            {
                map_prg16k(0, 0);
                map_prg16k(1, regs[3] & 0xF);
            }
        }
        // 32KB PRG:
        else
            map_prg32k((regs[3] & 0xF) >> 1);

        // 4KB CHR:
        if (regs[0] & 0b10000)
        {
            map_chr4k(0, regs[1]);
            map_chr4k(1, regs[2]);
        }
        // 8KB CHR:
        else
            map_chr8k(regs[1] >> 1);
    }

  public:
    Mapper1(u8* rom) : Mapper(rom)
    {
        regs[0] = 0x0C;
        writeN = tmpReg = regs[1] = regs[2] = regs[3] = 0;
        apply();
    }

    u8 write(u16 addr, u8 v)
    {
        if (addr < 0x8000)
        {
            prgRam[addr - 0x6000] = v;
        }
        else if (addr & 0x8000)
        {
            if (v & 0x80)
            {
                writeN   = 0;
                tmpReg   = 0;
                regs[0] |= 0x0C;
                apply();
            }
            else
            {
                tmpReg = ((v & 1) << 4) | (tmpReg >> 1);
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

    u8 chr_write(u16 addr, u8 v)
    {
        return chr[addr] = v;
    }
};
