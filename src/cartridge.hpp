#pragma once
#include "common.hpp"

namespace Cartridge {


template <bool wr> u8     access(u16 addr, u8 v = 0);
template <bool wr> u8 chr_access(u16 addr, u8 v = 0);
void signal_scanline();
void load(const char* fileName);
bool loaded();


}
