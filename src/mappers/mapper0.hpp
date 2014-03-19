#pragma once
#include "mapper.hpp"


class Mapper0 : public Mapper
{
  public:
    Mapper0(u8* rom) : Mapper(rom)
    {
        map_prg32k(0);
        map_chr8k(0);
    }
};
