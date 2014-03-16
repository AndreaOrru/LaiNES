#pragma once
#include "mapper.hpp"


class Mapper0 : public Mapper
{
  public:
    Mapper0(u8* rom) : Mapper(rom)
    {
        for (int i = 0; i < 8; i++)
            banksMap[i] = (i % (prgSize / 0x1000)) * 0x1000;
    }
};
