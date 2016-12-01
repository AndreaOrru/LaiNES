#pragma once
#include "mapper.hpp"


class Mapper0 : public Mapper
{
  public:
    Mapper0(u8* rom) : Mapper(rom)
    {
        map_prg<32>(0, 0);
        map_chr<8> (0, 0);
    }
};
