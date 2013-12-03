#include "common.hpp"

#ifndef CARTRIDGE_HPP
#define CARTRIDGE_HPP
namespace Cartridge {


template <bool wr> u8     access(u16 addr, u8 v = 0);
template <bool wr> u8 chr_access(u16 addr, u8 v = 0);
void load(const char* fname);


}
#endif  // CARTRIDGE_HPP
